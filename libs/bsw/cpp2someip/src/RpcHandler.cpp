/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/RpcHandler.h"

#include "someip/IEventReceiver.h"
#include "someip/INetwork.h"
#include "someip/IServiceRegistry.h"
#include "someip/ISomeIpSerializable.h"
#include "someip/NetworkChannel.h"
#include "someip/QueryManager.h"
#include "someip/ServiceHandler.h"
#include "someip/SomeIpConstants.h"
#include "someip/SomeIpParser.h"
#include "someip/SomeIpSerializer.h"
#include "someip/Statistics.h"
#include "someip/TpTransceiver.h"
#include "someip/logger.h"

#include <ip/to_str.h>

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace
{
class RequestContextCleanup final
{
public:
    using RpcCallback = ::someip::ServiceHandler::RpcCallback;

    explicit RequestContextCleanup(::someip::RequestContext& context) : _context(context) {}

    ~RequestContextCleanup()
    {
        _context.pService->releaseCallback(*static_cast<RpcCallback*>(_context.callback));
        _context.pResponse = nullptr;
        _context.pRequest  = nullptr;
    }

private:
    ::someip::RequestContext& _context;
};

} // anonymous namespace

namespace someip
{
using ::ip::IPAddress;
using ::ip::IPEndpoint;
using ::util::logger::SOMEIP;

RpcHandler::RpcHandler(
    INetwork& network,
    ::async::ContextType const ethernetContext,
    ITpTransceiver& tpTransceiver,
    ServiceManager& serviceManager,
    IServiceRegistry& serviceRegistry)
: _network(network)
, _ethernetContext(ethernetContext)
, _tpTransceiver(tpTransceiver)
, _serviceManager(serviceManager)
, _serviceRegistry(serviceRegistry)
, _pEventReceiver(nullptr)
{}

void RpcHandler::setEventReceiver(IEventReceiver& eventReceiver)
{
    _pEventReceiver = &eventReceiver;
}

void RpcHandler::removeEventReceiver() { _pEventReceiver = nullptr; }

RpcHandler::ErrorCode RpcHandler::handleRequest(
    SomeIpMessage const& message,
    IPEndpoint const& sourceAddress,
    port::type localPort,
    proto::type proto)
{
    RpcHandler::ErrorCode result     = ErrorCode::RPC_HANDLER_ERROR;
    service_id::type const serviceId = message.getServiceId();
    uint16_t const methodId          = message.getMethodId();
    uint8_t const interfaceVersion   = message.getInterfaceVersion();

    auto service         = ::someip::make<ServiceDescription>();
    service.serviceId    = serviceId;
    service.majorVersion = interfaceVersion;
    service.port         = localPort;
    service.proto        = proto;
    ServiceManager::FindServiceResult smResult
        = ServiceManager::FindServiceResult::FIND_SERVICE_UNKNOWN;

    ServiceHandler* const handler = _serviceManager.getHandler(service, smResult);

    switch (smResult)
    {
        case ServiceManager::FindServiceResult::FIND_SERVICE_OK:
        {
            if (handler == nullptr)
            {
                WARN_LOG(
                    SOMEIP,
                    "RpcHandler::handleRequest() no service handler (serviceId %d, port %d, proto "
                    "%s)",
                    serviceId,
                    localPort,
                    proto == proto::SD_L4_PROTO_UDP ? "UDP" : "TCP");
                result = RpcHandler::ErrorCode::RPC_HANDLER_ERROR;
            }
            else if (handler->getMethodDetail(methodId) == nullptr)
            {
                WARN_LOG(
                    SOMEIP,
                    "RpcHandler::handleRequest(): unknown method (serviceId %d, "
                    "interfaceVersion %d, port %d, proto %s, methodId %d)",
                    serviceId,
                    interfaceVersion,
                    localPort,
                    proto == proto::SD_L4_PROTO_UDP ? "UDP" : "TCP",
                    methodId);
                Statistics::incCounter(Statistics::Counter::RPC_UNKNOWN_METHOD_RX);
                result = ErrorCode::RPC_HANDLER_UNKNOWN_METHOD;
            }
            else if (
                static_cast<uint8_t>(handler->getMethodDetail(methodId)->callSemantic)
                != static_cast<uint8_t>(message.getMessageType()) + 1U)
            {
                switch (handler->getMethodDetail(methodId)->callSemantic)
                {
                    case SomeIpCallSemantic::SEMANTIC_REQUEST_RESPONSE:
                    {
                        return ErrorCode::RPC_HANDLER_WRONG_MESSAGE_TYPE_REQUEST_RESPONSE;
                    }
                    case SomeIpCallSemantic::SEMANTIC_FIRE_AND_FORGET:
                    {
                        return ErrorCode::RPC_HANDLER_WRONG_MESSAGE_TYPE_FIRE_AND_FORGET;
                    }
                    case SomeIpCallSemantic::SEMANTIC_UNKNOWN:
                    {
                        Statistics::incCounter(Statistics::Counter::RPC_MALFORMED_MESSAGE_RX);
                        return ErrorCode::RPC_HANDLER_WRONG_MESSAGE_TYPE;
                    }
                }
            }
            else if (!handler->hasAvailableCallback())
            {
                WARN_LOG(
                    SOMEIP,
                    "RpcHandler::handleRequest() number of parallel callbacks exhausted (serviceId "
                    "%d, "
                    "port %d, proto %s)",
                    serviceId,
                    localPort,
                    proto == proto::SD_L4_PROTO_UDP ? "UDP" : "TCP");
                result = RpcHandler::ErrorCode::RPC_HANDLER_ERROR;
            }
            else
            {
                ServiceHandler::RpcCallback& callback = handler->getCallback();
                RequestContext context;

                SomeIpParser parser(message.getBufferPayload());

                context.pRequest           = handler->createRequest(methodId, parser);
                context.pResponse          = handler->getResponse(methodId);
                context.pService           = handler;
                context.remoteIp           = sourceAddress;
                context.localPort          = localPort;
                context.proto              = proto;
                context.requestId          = message.getRequestId(); // client-id && session-id
                context.serviceId          = serviceId;
                context.methodId           = methodId;
                context.interfaceVersion   = interfaceVersion;
                context.callback           = static_cast<void*>(&callback);
                context.requestMessageType = message.getMessageType();

                callback
                    = ServiceHandler::RpcCallback::fromObject<RpcHandler, &RpcHandler::requestDone>(
                        *this, context);

                handler->dispatchMethod(methodId, context.pRequest, context.pResponse, callback);
                result = RpcHandler::ErrorCode::RPC_HANDLER_OK;
            }
            break;
        }
        case ServiceManager::FindServiceResult::FIND_SERVICE_WRONG_MAJOR_VERSION:
        {
            WARN_LOG(
                SOMEIP,
                "RpcHandler::handleRequest(): wrong interface version (serviceId %d, "
                "interfaceVersion "
                "%d, port %d, proto %s)",
                serviceId,
                interfaceVersion,
                localPort,
                proto == proto::SD_L4_PROTO_UDP ? "UDP" : "TCP");
            Statistics::incCounter(Statistics::Counter::RPC_WRONG_INTERFACE_VERSION_RX);
            result = RpcHandler::ErrorCode::RPC_HANDLER_WRONG_INTERFACE_VERSION;
            break;
        }
        case ServiceManager::FindServiceResult::FIND_SERVICE_UNKNOWN:
        default:
        {
            WARN_LOG(
                SOMEIP,
                "RpcHandler::handleRequest(): no service provided (serviceId %d, port %d, proto "
                "%s)",
                serviceId,
                localPort,
                proto == proto::SD_L4_PROTO_UDP ? "UDP" : "TCP");
            Statistics::incCounter(Statistics::Counter::RPC_UNKNOWN_SERVICE_RX);
            result = ErrorCode::RPC_HANDLER_UNKNOWN_SERVICE;
            break;
        }
    }
    return result;
}

IRpcHandler::ErrorCode
RpcHandler::handleMessage(NetworkChannel const& channel, SomeIpMessage const& message)
{
    IRpcHandler::ErrorCode errorCode = IRpcHandler::ErrorCode::RPC_HANDLER_OK;

    service_id::type const serviceId       = message.getServiceId();
    major_version::type const majorVersion = message.getInterfaceVersion();
    uint16_t const methodId                = message.getMethodId();
    instance_id::type instanceId           = instance_id::ANY;

    if (message.getMessageType() == SomeIpMessage::MessageType::REQUEST
        || message.getMessageType() == SomeIpMessage::MessageType::REQUEST_NO_RETURN)
    {
        // Local provider
        auto const portResult = channel.getLocalPort();
        if (!portResult.has_value())
        {
            WARN_LOG(SOMEIP, "RpcHandler: unable to get local port");
            return ErrorCode::RPC_HANDLER_WRONG_MESSAGE_TYPE;
        }

        instanceId = _serviceRegistry.getInstanceId(
            message.getServiceId(),
            message.getInterfaceVersion(),
            _network.getLocalIp(),
            portResult.value(),
            false); // false means local provider
    }
    else
    {
        // Remote provider
        instanceId = _serviceRegistry.getInstanceId(
            message.getServiceId(),
            message.getInterfaceVersion(),
            channel.getRemoteEndpoint().getAddress(),
            channel.getRemoteEndpoint().getPort());
    }
    DEBUG_LOG(
        SOMEIP,
        "RpcHandler::handleMessage(serviceId %d, methodId %d, instanceId %d, majorVersion "
        "%d)",
        serviceId,
        methodId,
        instanceId,
        majorVersion);

    bool const queryManagerCheck = _serviceRegistry.getQueryManager() != nullptr;
    bool const notificationCheck
        = message.getMessageType() != SomeIpMessage::MessageType::NOTIFICATION;

    if (queryManagerCheck && notificationCheck)
    {
        ServiceQuery const* const query
            = _serviceRegistry.getQueryManager()->getQuery(serviceId, instanceId);

        bool const nullptrCheck = (query != nullptr) && (query->listener != nullptr);

        bool const msgTypeCheck
            = nullptrCheck
              && (static_cast<uint8_t>(query->listener->getMethodDetail(methodId)->callSemantic)
                  != static_cast<uint8_t>(message.getMessageType()) + 1U);

        if (msgTypeCheck)
        {
            switch (query->listener->getMethodDetail(methodId)->callSemantic)
            {
                case SomeIpCallSemantic::SEMANTIC_REQUEST_RESPONSE:
                {
                    errorCode = ErrorCode::RPC_HANDLER_WRONG_MESSAGE_TYPE_REQUEST_RESPONSE;
                    break;
                }
                case SomeIpCallSemantic::SEMANTIC_FIRE_AND_FORGET:
                {
                    errorCode = ErrorCode::RPC_HANDLER_WRONG_MESSAGE_TYPE_FIRE_AND_FORGET;
                    break;
                }
                case SomeIpCallSemantic::SEMANTIC_UNKNOWN:
                {
                    errorCode = ErrorCode::RPC_HANDLER_WRONG_MESSAGE_TYPE;
                    break;
                }
            }
        }
    }

    auto const localPortResult = channel.getLocalPort();
    if (!localPortResult.has_value())
    {
        WARN_LOG(SOMEIP, "RpcHandler::handleMessage() no local port available");
        return ErrorCode::RPC_HANDLER_WRONG_MESSAGE_TYPE;
    }
    auto const localPort = localPortResult.value();

    switch (message.getMessageType())
    {
        case SomeIpMessage::MessageType::REQUEST:
        case SomeIpMessage::MessageType::REQUEST_NO_RETURN:
        {
            errorCode = handleRequest(
                message, channel.getRemoteEndpoint(), localPort, channel.getProto());
            break;
        }
        case SomeIpMessage::MessageType::RESPONSE:
        {
            errorCode = handleResponse(message, channel.getRemoteEndpoint(), localPort);
            break;
        }
        case SomeIpMessage::MessageType::NOTIFICATION:
        {
            if (message.getMessageId() != SD_MESSAGE_ID)
            {
                handleNotification(message, channel.getRemoteEndpoint(), localPort);
            }
            else
            {
                WARN_LOG(SOMEIP, "RpcHandler::handleMessage(): dropping SD message");
            }
            break;
        }
        case SomeIpMessage::MessageType::EXCEPTION:
        {
            (void)handleError(message, channel.getRemoteEndpoint());
            errorCode = IRpcHandler::ErrorCode::RPC_HANDLER_OK; // ignore errors
            break;
        }
        default:
        {
            WARN_LOG(
                SOMEIP,
                "RpcHandler::handleMessage(): dropping message (serviceId 0x%x, messageType 0x%x)",
                message.getServiceId(),
                message.getMessageType());
            errorCode = IRpcHandler::ErrorCode::RPC_HANDLER_WRONG_MESSAGE_TYPE;
            break;
        }
    }

    return errorCode;
}

RpcHandler::ErrorCode RpcHandler::handleResponse(
    SomeIpMessage const& message, IPEndpoint const& sourceAddress, port::type const localPort)
{
    RpcHandler::ErrorCode result     = RpcHandler::ErrorCode::RPC_HANDLER_ERROR;
    service_id::type const serviceId = message.getServiceId();
    uint16_t const clientId          = static_cast<uint16_t>(message.getClientId());
    uint16_t const methodId          = message.getMethodId();
    uint32_t const payloadLength     = message.getPayloadLength();
    INFO_LOG(
        SOMEIP,
        "RpcHandler::handleResponse(serviceId %d, clientId %d, methodId %d, %d bytes)",
        serviceId,
        clientId,
        methodId,
        payloadLength);

    IRpcChannel* const pChannel
        = findChannel(serviceId, clientId, sourceAddress, message.getSessionId());
    if (pChannel == nullptr)
    {
        WARN_LOG(SOMEIP, "RpcHandler::handleResponse(): no channel waiting");
        result = RpcHandler::ErrorCode::RPC_HANDLER_NOT_RESPONSIBLE;
    }
    else if (pChannel->getLocalPort() != localPort)
    {
        WARN_LOG(SOMEIP, "RpcHandler::handleResponse(): wrong local port");
        result = RpcHandler::ErrorCode::RPC_HANDLER_ERROR;
    }
    else
    {
        unregisterChannel(*pChannel);
        pChannel->cancelTimeout();
        ISomeIpSerializable* const pResponse = pChannel->getResponse();
        if (pResponse != nullptr)
        {
            SomeIpParser parser(
                ::etl::span<uint8_t const>(message.getPayload(), message.getPayloadLength()));
            pResponse->parseFromArray(parser);
            if (parser.isGood() == false)
            {
                WARN_LOG(SOMEIP, "RpcHandler::handleResponse(): invalid payload");
                pChannel->responseReceived(::someip::RPC_INVALID_PAYLOAD);
            }
        }

        pChannel->responseReceived(getRpcErrorCode(
            static_cast<typename SomeIpMessage::ReturnCode>(message.getReturnCode())));

        result = RpcHandler::ErrorCode::RPC_HANDLER_OK;
    }

    return result;
}

void RpcHandler::handleNotification(
    SomeIpMessage const& message, IPEndpoint const& sourceAddress, port::type const localPort)
{
    uint32_t const messageId = message.getMessageId();
    if (messageId != SD_MESSAGE_ID)
    {
        service_id::type const serviceId       = message.getServiceId();
        major_version::type const majorVersion = message.getInterfaceVersion();
        uint16_t const eventId                 = message.getMethodId();
        instance_id::type instanceId           = _serviceRegistry.getInstanceId(
            serviceId, majorVersion, sourceAddress.getAddress(), sourceAddress.getPort());

        DEBUG_LOG(
            SOMEIP,
            "RpcHandler::handleNotification(serviceId %d, eventId %d, instanceId %d, majorVersion "
            "%d)",
            serviceId,
            eventId,
            instanceId,
            majorVersion);

        if (instanceId == instance_id::ANY)
        {
            char addressStr[::ip::MAX_ENDPOINT_STRING_LENGTH];
            char* const addressStrPtr = ::ip::to_str(sourceAddress, addressStr).data();
            WARN_LOG(
                SOMEIP,
                "Invalid InstanceId for serviceId: %d majorVersion: %d address: %s",
                serviceId,
                majorVersion,
                addressStrPtr);
            return;
        }

        if (!_serviceRegistry.isEventgroupPort(serviceId, instanceId, majorVersion, localPort))
        {
            WARN_LOG(SOMEIP, "RpcHandler::handleNotification(): invalid local port");
            return;
        }

        if (_pEventReceiver != nullptr)
        {
            SomeIpParser parser(
                ::etl::span<uint8_t const>(message.getPayload(), message.getPayloadLength()));
            _pEventReceiver->eventReceived(serviceId, eventId, instanceId, majorVersion, parser);
        }
        else
        {
            WARN_LOG(SOMEIP, "RpcHandler::handleNotification(): no event receiver registered");
        }
        return;
    }

    WARN_LOG(SOMEIP, "RpcHandler::handleNotification(): invalid messageId: 0x%x", messageId);
}

RpcHandler::ErrorCode
RpcHandler::handleError(SomeIpMessage const& message, IPEndpoint const& sourceAddress)
{
    service_id::type const serviceId           = message.getServiceId();
    uint16_t const clientId                    = static_cast<uint16_t>(message.getClientId());
    uint16_t const methodId                    = message.getMethodId();
    SomeIpMessage::ReturnCode const returnCode = message.getReturnCode();

    INFO_LOG(
        SOMEIP,
        "RpcHandler::handleError(serviceId %d, clientId %d, methodId %d, error 0x%x)",
        serviceId,
        clientId,
        methodId,
        returnCode);

    IRpcChannel* const pChannel
        = findChannel(serviceId, clientId, sourceAddress, message.getSessionId());
    if (pChannel != nullptr)
    {
        unregisterChannel(*pChannel);
        pChannel->cancelTimeout();
        pChannel->responseReceived(getRpcErrorCode(returnCode));
        return RpcHandler::ErrorCode::RPC_HANDLER_OK;
    }

    WARN_LOG(SOMEIP, "RpcHandler::handleError(): no channel waiting");
    return RpcHandler::ErrorCode::RPC_HANDLER_ERROR;
}

ServiceResultCode RpcHandler::getRpcErrorCode(SomeIpMessage::ReturnCode const error)
{
    switch (error)
    {
        case SomeIpMessage::ReturnCode::SOMEIP_E_OK:
        {
            return ::someip::RPC_POSITIVE_RESPONSE;
        }
        case SomeIpMessage::ReturnCode::SOMEIP_E_NOT_OK:
        {
            return ::someip::RPC_UNDEFINED_ERROR;
        }
        case SomeIpMessage::ReturnCode::SOMEIP_E_UNKNOWN_SERVICE:
        {
            return ::someip::RPC_SERVICE_NOT_AVAILABLE;
        }
        case SomeIpMessage::ReturnCode::SOMEIP_E_UNKNOWN_METHOD:
        {
            return ::someip::RPC_METHOD_NOT_AVAILABLE;
        }
        case SomeIpMessage::ReturnCode::SOMEIP_E_WRONG_PROTOCOL_VERSION:
        {
            return ::someip::RPC_WRONG_PROTOCOL_VERSION;
        }
        case SomeIpMessage::ReturnCode::SOMEIP_E_WRONG_INTERFACE_VERSION:
        {
            return ::someip::RPC_WRONG_INTERFACE_VERSION;
        }
        case SomeIpMessage::ReturnCode::SOMEIP_E_MALFORMED_MESSAGE:
        {
            return ::someip::RPC_INVALID_PAYLOAD;
        }
        default:
        {
            return ::someip::RPC_UNDEFINED_ERROR;
        }
    }
}

SomeIpMessage::ReturnCode RpcHandler::getSomeipErrorCode(ServiceResultCode const error)
{
    switch (error)
    {
        case ::someip::RPC_SERVICE_NOT_AVAILABLE:
        {
            return SomeIpMessage::ReturnCode::SOMEIP_E_UNKNOWN_SERVICE;
        }
        case ::someip::RPC_METHOD_NOT_AVAILABLE:
        {
            return SomeIpMessage::ReturnCode::SOMEIP_E_UNKNOWN_METHOD;
        }
        case ::someip::RPC_INVALID_PAYLOAD:
        {
            return SomeIpMessage::ReturnCode::SOMEIP_E_MALFORMED_MESSAGE;
        }
        default:
        {
            return SomeIpMessage::ReturnCode::SOMEIP_E_NOT_OK;
        }
    }
}

ServiceResultCode RpcHandler::sendRequest(
    ISomeIpSerializable const* pRequest,
    service_id::type const serviceId,
    uint16_t const methodId,
    uint8_t const interfaceVersion,
    bool const isResponseExpected,
    IRpcChannel& channel,
    uint32_t timeout)
{
    auto const localPortResult = channel.getLocalPort();
    if (!localPortResult.has_value())
    {
        WARN_LOG(SOMEIP, "RpcHandler::sendRequest() no local port available");
        return COULD_NOT_DELIVER;
    }

    auto const localPort = localPortResult.value();
    auto const remoteIp  = channel.getRemoteIp();
    auto const proto     = channel.getProto();
    auto networkChannel  = _network.getRpcChannel(localPort, remoteIp, proto);
    auto const sessionId = channel.getSessionId();
    auto const clientId  = channel.getClientId();

    if (!networkChannel.has_value())
    {
        WARN_LOG(SOMEIP, "RpcHandler::sendRequest() no channel");
        return COULD_NOT_DELIVER;
    }

    auto const output = networkChannel->getOutputBuffer();

    if (SomeIpMessage::OFFSET_PAYLOAD > output.size())
    {
        ERROR_LOG(SOMEIP, "RpcHandler::sendRequest(): buffer too small for message header");
        return COULD_NOT_DELIVER;
    }

    SomeIpMessage message(output);
    message.setServiceId(serviceId);
    message.setMethodId(methodId);
    message.setClientId(clientId);
    message.setSessionId(sessionId);
    message.setPayloadLength(0U);
    message.setProtocolVersion(::someip::configuration::PROTOCOL_VERSION);
    message.setInterfaceVersion(interfaceVersion);
    message.setReturnCode(SomeIpMessage::ReturnCode::SOMEIP_E_OK);

    if (pRequest != nullptr)
    {
        SomeIpSerializer serializer(
            ::etl::span<uint8_t>(message.getPayload(), message.getMaximumPayloadLength()));
        pRequest->serializeToArray(serializer);
        if (serializer.isGood() == false)
        {
            WARN_LOG(SOMEIP, "RpcHandler::sendRequest(): invalid payload");
            return COULD_NOT_DELIVER;
        }
        message.setPayloadLength(static_cast<uint32_t>(serializer.getCurrentPosition()));
    }
    if (isResponseExpected)
    {
        message.setMessageType(SomeIpMessage::MessageType::REQUEST);

        registerChannel(channel);
        channel.setTimeout(_ethernetContext, timeout);
    }
    else
    {
        message.setMessageType(SomeIpMessage::MessageType::REQUEST_NO_RETURN);
    }

    bool result                    = false;
    auto const networkChannelProto = networkChannel->getProto();
    auto const messageTotalLength  = message.getTotalLength();
    if (ITpTransceiver::isOutgoingTpMessage(networkChannelProto, messageTotalLength))
    {
        result = _tpTransceiver.sendTpMessage(*networkChannel, message);
    }
    else
    {
        result = networkChannel->send(messageTotalLength);
    }

    if (!result)
    {
        WARN_LOG(SOMEIP, "RpcHandler::sendRequest() send failed");
        if (isResponseExpected)
        {
            unregisterChannel(channel);
            channel.cancelTimeout();
        }

        return COULD_NOT_DELIVER;
    }

    Statistics::incCounter(Statistics::Counter::PDU_TX);
    return RPC_SENT_SUCCESSFULLY;
}

void RpcHandler::requestExpired(IRpcChannel& channel) { unregisterChannel(channel); }

ServiceResultCode RpcHandler::requestDone(RequestContext& context, ServiceResultCode const result)
{
    // make sure we cleanup after ourselves when we are done
    RequestContextCleanup const cleanup(context);

    if ((result != ::someip::RPC_POSITIVE_RESPONSE)
        && (context.requestMessageType != SomeIpMessage::MessageType::REQUEST_NO_RETURN))
    {
        return sendError(
            context.requestId,
            context.serviceId,
            context.methodId,
            context.interfaceVersion,
            static_cast<uint8_t>(getSomeipErrorCode(result)),
            context.localPort,
            context.proto,
            context.remoteIp);
    }

    if (SomeIpMessage::MessageType::REQUEST == context.requestMessageType)
    {
        auto channel = _network.getRpcChannel(context.localPort, context.remoteIp, context.proto);

        if (!channel.has_value())
        {
            WARN_LOG(SOMEIP, "RpcHandler::requestDone() no channel");
            return COULD_NOT_DELIVER;
        }

        auto const output = channel->getOutputBuffer();

        SomeIpMessage message(output);
        message.setRequestId(context.requestId);
        message.setServiceId(context.serviceId);
        message.setMethodId(context.methodId);
        message.setMessageType(SomeIpMessage::MessageType::RESPONSE);
        message.setProtocolVersion(::someip::configuration::PROTOCOL_VERSION);
        message.setInterfaceVersion(context.interfaceVersion);
        message.setReturnCode(static_cast<SomeIpMessage::ReturnCode>(context.operationResult));

        ISomeIpSerializable* const pResponse = context.pResponse;
        if (pResponse != nullptr)
        {
            SomeIpSerializer serializer(
                ::etl::span<uint8_t>(message.getPayload(), message.getMaximumPayloadLength()));
            pResponse->serializeToArray(serializer);
            if (serializer.isGood() == false)
            {
                WARN_LOG(SOMEIP, "RpcHandler::requestDone() invalid payload");
                return COULD_NOT_DELIVER;
            }
            message.setPayloadLength(static_cast<uint32_t>(serializer.getCurrentPosition()));
        }
        else
        {
            message.setPayloadLength(0U);
        }

        uint32_t const length = message.getTotalLength();

        bool tpMessageSent;
        if (ITpTransceiver::isOutgoingTpMessage(channel->getProto(), length))
        {
            tpMessageSent = _tpTransceiver.sendTpMessage(*channel, message);
        }
        else
        {
            tpMessageSent = channel->send(length);
        }

        if (!tpMessageSent)
        {
            WARN_LOG(SOMEIP, "RpcHandler::requestDone() send failed");
            return COULD_NOT_DELIVER;
        }

        Statistics::incCounter(Statistics::Counter::PDU_TX);
    }

    return RPC_SENT_SUCCESSFULLY;
}

ServiceResultCode RpcHandler::sendError(
    uint32_t const requestId,
    service_id::type const serviceId,
    uint16_t const methodId,
    uint8_t const interfaceVersion,
    uint8_t const returnCode,
    port::type const localPort,
    proto::type const proto,
    IPEndpoint const& remoteIp) const
{
    auto channel = _network.getRpcChannel(localPort, remoteIp, proto);

    if (!channel.has_value())
    {
        WARN_LOG(SOMEIP, "RpcHandler::sendError() no channel");
        return COULD_NOT_DELIVER;
    }

    auto const output = channel->getOutputBuffer();

    SomeIpMessage message(output);
    message.setMessageType(SomeIpMessage::MessageType::EXCEPTION);
    message.setRequestId(requestId);
    message.setServiceId(serviceId);
    message.setMethodId(methodId);
    message.setPayloadLength(0U);
    message.setProtocolVersion(::someip::configuration::PROTOCOL_VERSION);
    message.setInterfaceVersion(interfaceVersion);
    message.setReturnCode(static_cast<SomeIpMessage::ReturnCode>(returnCode));

    uint32_t const length = message.getTotalLength();
    if (length > output.size())
    {
        ERROR_LOG(
            SOMEIP,
            "RpcHandler::sendError() length %d exceeds max of %d bytes",
            length,
            output.size());
        return COULD_NOT_DELIVER;
    }

    bool const result = channel->send(length);

    if (!result)
    {
        WARN_LOG(SOMEIP, "RpcHandler::sendError() send failed");
    }

    return result ? RPC_SENT_SUCCESSFULLY : COULD_NOT_DELIVER;
}

size_t RpcHandler::getNumRegisteredChannels() const { return _rpcChannelList.size(); }

void RpcHandler::registerChannel(IRpcChannel& channel)
{
    if (!_rpcChannelList.contains_node(channel))
    {
        _rpcChannelList.push_front(channel);
    }
    else
    {
        WARN_LOG(SOMEIP, "RpcHandler::registerChannel() channel already registered!");
    }
}

// private
void RpcHandler::unregisterChannel(IRpcChannel& channel)
{
    if (_rpcChannelList.contains_node(channel))
    {
        _rpcChannelList.remove_if([&channel](IRpcChannel const& ch) { return &ch == &channel; });
    }
}

// private
IRpcChannel* RpcHandler::findChannel(
    service_id::type const serviceId,
    uint16_t const clientId,
    IPEndpoint const& remoteIp,
    uint16_t const sessionId)
{
    for (auto& itr : _rpcChannelList)
    {
        if ((itr.getServiceId() == serviceId) && (itr.getClientId() == clientId)
            && (itr.getRemoteIp() == remoteIp) && (itr.getSessionId() == sessionId))
        {
            return &itr;
        }
    }

    return nullptr;
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
