/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/RpcReceiver.h"

#include "someip/INetwork.h"
#include "someip/IServiceRegistry.h"
#include "someip/SomeIpConstants.h"
#include "someip/SomeIpMessage.h"
#include "someip/Statistics.h"
#include "someip/logger.h"

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::util::logger::SOMEIP;

using ::ip::IPAddress;
using ::ip::IPEndpoint;

RpcReceiver::RpcReceiver(
    INetwork& network,
    ITpTransceiver& tpTransceiver,
    IEventReceiver& eventReceiver,
    IServiceRegistry& serviceRegistry,
    IRpcHandler& rpcHandler,
    MulticastReceptionList& multicastReceptionList,
    IDiagnosticListener* const diagnosticListener)
: _network(network)
, _tpTransceiver(tpTransceiver)
, _eventReceiver(eventReceiver)
, _pDiagnosticListener(diagnosticListener)
, _serviceRegistry(serviceRegistry)
, _rpcHandler(rpcHandler)
, _pPriorityRpcHandler(nullptr)
, _multicastReceptions(multicastReceptionList)
{
    (void)_serviceRegistry;
    _network.setRpcListener(*this);
}

void RpcReceiver::init() { _rpcHandler.setEventReceiver(_eventReceiver); }

void RpcReceiver::shutdown() { _rpcHandler.removeEventReceiver(); }

// virtual
void RpcReceiver::received(NetworkChannel& channel, uint32_t const length)
{
    Statistics::incCounter(Statistics::Counter::FRAME_RX);

    ::etl::span<uint8_t> const input = channel.getInputBuffer();

    uint32_t offset = 0U;
    while (offset < length)
    {
        uint32_t const bytesLeft = length - offset;
        if (bytesLeft < SomeIpMessage::OFFSET_PAYLOAD)
        {
            ERROR_LOG(
                SOMEIP,
                "RpcReceiver::dataReceived(): %d bytes left, but no valid header",
                bytesLeft);
            Statistics::incCounter(Statistics::Counter::RPC_MALFORMED_MESSAGE_RX);
            return;
        }

        Statistics::incCounter(Statistics::Counter::PDU_RX);

        SomeIpMessage const message(input.subspan(offset, bytesLeft));

        uint32_t const payloadLength = message.getPayloadLength();
        if (payloadLength <= bytesLeft - SomeIpMessage::OFFSET_PAYLOAD)
        {
            offset += (payloadLength + SomeIpMessage::OFFSET_PAYLOAD);
        }
        else
        {
            ERROR_LOG(
                SOMEIP,
                "RpcReceiver::dataReceived(): %d bytes left, but expected is %d",
                bytesLeft,
                payloadLength + SomeIpMessage::OFFSET_PAYLOAD);
            Statistics::incCounter(Statistics::Counter::RPC_MALFORMED_MESSAGE_RX);
            if ((message.getMessageType() != SomeIpMessage::MessageType::REQUEST_NO_RETURN)
                && (message.getMessageType() != SomeIpMessage::MessageType::NOTIFICATION))
            {
                sendError(channel, message, SomeIpMessage::ReturnCode::SOMEIP_E_MALFORMED_MESSAGE);
            }
            return;
        }

        if ((message.getReturnCode() != SomeIpMessage::ReturnCode::SOMEIP_E_OK)
            && (message.getMessageType() != SomeIpMessage::MessageType::EXCEPTION))
        {
            WARN_LOG(
                SOMEIP,
                "RpcReceiver::dataReceived(): invalid return code: %d",
                message.getReturnCode());
            return; // drop incoming SOME/IP error message
        }

        if (message.getProtocolVersion() != configuration::PROTOCOL_VERSION)
        {
            WARN_LOG(
                SOMEIP,
                "RpcReceiver::dataReceived(): invalid protocol version %d",
                message.getProtocolVersion());
            Statistics::incCounter(Statistics::Counter::RPC_WRONG_PROTOCOL_VERSION_RX);
            if ((message.getMessageType() != SomeIpMessage::MessageType::REQUEST_NO_RETURN)
                && (message.getMessageType() != SomeIpMessage::MessageType::NOTIFICATION))
            {
                sendError(
                    channel, message, SomeIpMessage::ReturnCode::SOMEIP_E_WRONG_PROTOCOL_VERSION);
            }
            continue; // drop message
        }

        if (isMagicCookie(message))
        {
            DEBUG_LOG(
                SOMEIP,
                "RpcReceiver::dataReceived(): ignore magic-cookie (0x%X)",
                message.getMessageId());
            continue; // NYI: ignore message
        }

        if (ITpTransceiver::isIncomingTpMessage(channel.getProto(), message.getMessageType()))
        {
            _tpTransceiver.receiveTpMessage(channel, message, *this);
        }
        else
        {
            handleMessage(channel, message);
        }
    }
}

// virtual
void RpcReceiver::receivedTpMessage(NetworkChannel& channel, SomeIpMessage const& message)
{
    if ((message.getReturnCode() != SomeIpMessage::ReturnCode::SOMEIP_E_OK)
        && (message.getMessageType() != SomeIpMessage::MessageType::EXCEPTION))
    {
        WARN_LOG(
            SOMEIP,
            "RpcReceiver::receivedTpMessage(): invalid return code: %d",
            message.getReturnCode());
        return; // drop incoming SOME/IP error message
    }

    handleMessage(channel, message);
}

// virtual
void RpcReceiver::setPriorityRpcHandler(IRpcHandler& priorityRpcHandler)
{
    _pPriorityRpcHandler = &priorityRpcHandler;
}

// virtual
void RpcReceiver::removePriorityRpcHandler() { _pPriorityRpcHandler = nullptr; }

// virtual
bool RpcReceiver::requestMulticastReception(IPEndpoint const& multicastEndpoint)
{
    uint8_t const ip0   = multicastEndpoint.getAddress().raw[0];
    uint8_t const ip1   = multicastEndpoint.getAddress().raw[1];
    uint8_t const ip2   = multicastEndpoint.getAddress().raw[2];
    uint8_t const ip3   = multicastEndpoint.getAddress().raw[3];
    uint16_t const port = multicastEndpoint.getPort();

    if (_multicastReceptions.full())
    {
        WARN_LOG(
            SOMEIP,
            "RpcReceiver::requestMulticastReception() multicast pool empty. Unable to add "
            "(%d,%d,%d,%d), Port: %d.",
            ip0,
            ip1,
            ip2,
            ip3,
            port);
        return true;
    }

    // Check if already exists
    internal::FindNetworkChannelCondition const condition(
        multicastEndpoint.getAddress(), multicastEndpoint.getPort());
    if (::etl::find_if(_multicastReceptions.begin(), _multicastReceptions.end(), condition)
        != _multicastReceptions.end())
    {
        INFO_LOG(
            SOMEIP,
            "RpcReceiver::requestMulticastReception() channel already present for (%d,%d,%d,%d), "
            "Port: %d.",
            ip0,
            ip1,
            ip2,
            ip3,
            port);
        return true;
    }

    // Open a new channel
    auto const channel = _network.openUdpChannel(multicastEndpoint.getPort(), multicastEndpoint);

    if (!channel.has_value())
    {
        ERROR_LOG(
            SOMEIP,
            "RpcReceiver::requestMulticastReception() no channel. Unable to add (%d,%d,%d,%d), "
            "Port: %d.",
            ip0,
            ip1,
            ip2,
            ip3,
            port);
        return false;
    }

    // Insert the new channel
    auto insertResult = _multicastReceptions.insert(channel);
    if (!insertResult.second)
    {
        WARN_LOG(
            SOMEIP,
            "RpcReceiver::requestMulticastReception() not able to add channel for "
            "(%d,%d,%d,%d), Port: %d.",
            ip0,
            ip1,
            ip2,
            ip3,
            port);
        return false;
    }

    INFO_LOG(
        SOMEIP,
        "RpcReceiver::requestMulticastReception() channel added for (%d,%d,%d,%d), "
        "Port: %d.",
        ip0,
        ip1,
        ip2,
        ip3,
        port);
    return true;
}

// virtual
void RpcReceiver::cancelMulticastReception(IPEndpoint const& multicastEndpoint)
{
    auto channel = _network.getRpcChannel(
        multicastEndpoint.getPort(), multicastEndpoint, proto::SD_L4_PROTO_UDP);

    if (!channel.has_value())
    {
        WARN_LOG(SOMEIP, "RpcReceiver::cancelMulticastReception() no channel");
    }
    else
    {
        channel->close();
        internal::FindNetworkChannelCondition const condition(
            multicastEndpoint.getAddress(), multicastEndpoint.getPort());
        auto it = _multicastReceptions.begin();
        while (it != _multicastReceptions.end())
        {
            if (condition(*it))
            {
                _multicastReceptions.erase(it);
                break; // Found and removed the matching channel
            }

            ++it;
        }
    }
}

// private
// static
bool RpcReceiver::isMagicCookie(SomeIpMessage const& message)
{
    uint32_t const messageId = message.getMessageId();
    uint32_t const requestId = message.getRequestId();

    return ((MAGIC_COOKIE_CLIENT_MESSAGE_ID == messageId)
            || (MAGIC_COOKIE_SERVER_MESSAGE_ID == messageId))
           && (MAGIC_COOKIE_REQUEST_ID == requestId);
}

// private
void RpcReceiver::handleMessage(NetworkChannel& channel, SomeIpMessage const& message)
{
    IRpcHandler::ErrorCode errorCode = IRpcHandler::ErrorCode::RPC_HANDLER_NOT_RESPONSIBLE;

    if (_pPriorityRpcHandler != nullptr)
    {
        errorCode = _pPriorityRpcHandler->handleMessage(channel, message);
    }

    if (errorCode == IRpcHandler::ErrorCode::RPC_HANDLER_NOT_RESPONSIBLE)
    {
        errorCode = _rpcHandler.handleMessage(channel, message);
    }

    if ((errorCode != IRpcHandler::ErrorCode::RPC_HANDLER_OK)
        && (errorCode != IRpcHandler::ErrorCode::RPC_HANDLER_NOT_RESPONSIBLE))
    {
        if ((message.getMessageType() != SomeIpMessage::MessageType::REQUEST_NO_RETURN)
            && ((message.getMessageType() != SomeIpMessage::MessageType::NOTIFICATION)
                || (errorCode
                    != IRpcHandler::ErrorCode::RPC_HANDLER_WRONG_MESSAGE_TYPE_REQUEST_RESPONSE))
            && (errorCode
                != IRpcHandler::ErrorCode::RPC_HANDLER_WRONG_MESSAGE_TYPE_FIRE_AND_FORGET))
        {
            sendError(channel, message, getSomeipErrorCode(errorCode));
        }
    }
}

// private
void RpcReceiver::sendError(
    NetworkChannel& channel,
    SomeIpMessage const& message,
    SomeIpMessage::ReturnCode const returnCode)
{
    DEBUG_LOG(SOMEIP, "RpcReceiver::sendError(0x%x)", returnCode);

    SomeIpMessage error(channel.getOutputBuffer());
    error.setMessageType(SomeIpMessage::MessageType::EXCEPTION);
    error.setRequestId(message.getRequestId());
    error.setServiceId(message.getServiceId());
    error.setMethodId(message.getMethodId());
    error.setPayloadLength(0U);
    error.setProtocolVersion(::someip::configuration::PROTOCOL_VERSION);
    error.setInterfaceVersion(message.getInterfaceVersion());
    error.setReturnCode(returnCode);

    uint16_t const length = static_cast<uint16_t>(error.getTotalLength());
    if (!channel.send(length))
    {
        WARN_LOG(SOMEIP, "SomeIpSdTransceiver: send failed");
    }

    if (_pDiagnosticListener != nullptr)
    {
        _pDiagnosticListener->onError(
            channel.getRemoteEndpoint(),
            message,
            static_cast<SomeIpMessage::ReturnCode>(returnCode));
    }
}

// static
SomeIpMessage::ReturnCode RpcReceiver::getSomeipErrorCode(IRpcHandler::ErrorCode const error)
{
    switch (error)
    {
        case IRpcHandler::ErrorCode::RPC_HANDLER_OK:
        {
            return SomeIpMessage::ReturnCode::SOMEIP_E_OK;
        }
        case IRpcHandler::ErrorCode::RPC_HANDLER_UNKNOWN_SERVICE:
        {
            return SomeIpMessage::ReturnCode::SOMEIP_E_UNKNOWN_SERVICE;
        }
        case IRpcHandler::ErrorCode::RPC_HANDLER_UNKNOWN_METHOD:
        {
            return SomeIpMessage::ReturnCode::SOMEIP_E_UNKNOWN_METHOD;
        }
        case IRpcHandler::ErrorCode::RPC_HANDLER_WRONG_INTERFACE_VERSION:
        {
            return SomeIpMessage::ReturnCode::SOMEIP_E_WRONG_INTERFACE_VERSION;
        }
        case IRpcHandler::ErrorCode::RPC_HANDLER_MALFORMED_MESSAGE:
        {
            return SomeIpMessage::ReturnCode::SOMEIP_E_MALFORMED_MESSAGE;
        }
        case IRpcHandler::ErrorCode::RPC_HANDLER_WRONG_MESSAGE_TYPE:
        {
            return SomeIpMessage::ReturnCode::SOMEIP_E_WRONG_MESSAGE_TYPE;
        }
        default:
        {
            return SomeIpMessage::ReturnCode::SOMEIP_E_NOT_OK;
        }
    }
}

namespace internal
{

bool NetworkChannelComparator::operator()(
    ::etl::optional<NetworkChannel> const& lhs, ::etl::optional<NetworkChannel> const& rhs) const
{
    // Handle empty optionals (though they shouldn't be inserted)
    if (!lhs.has_value() && !rhs.has_value())
    {
        return false;
    }
    if (!lhs.has_value())
    {
        return true; // Empty sorts before non-empty
    }
    if (!rhs.has_value())
    {
        return false; // Non-empty sorts after empty
    }

    // Lexicographic comparison for strict weak ordering
    auto const& lhs_addr = lhs.value().getRemoteEndpoint().getAddress();
    auto const& rhs_addr = rhs.value().getRemoteEndpoint().getAddress();

#ifdef PLATFORM_SUPPORT_IPV6
    // IPv6-mapped IPv4 addresses: compare bytes 12-15
    for (size_t i = 12; i < 16; ++i)
#else
    // Pure IPv4: compare bytes 0-3
    for (size_t i = 0; i < 4; ++i)
#endif
    {
        if (lhs_addr.raw[i] != rhs_addr.raw[i])
        {
            return lhs_addr.raw[i] < rhs_addr.raw[i];
        }
    }

    auto const lhs_port = lhs.value().getLocalPort();
    auto const rhs_port = rhs.value().getLocalPort();

    if (!lhs_port.has_value() || !rhs_port.has_value())
    {
        return false;
    }

    return lhs_port.value() < rhs_port.value();
}

FindNetworkChannelCondition::FindNetworkChannelCondition(
    ::ip::IPAddress const& ipAddr, uint16_t const port)
: _ip(ipAddr), _port(port)
{}

bool FindNetworkChannelCondition::operator()(::etl::optional<NetworkChannel> const& channel) const
{
    if (!channel.has_value())
    {
        return false;
    }
    auto const address = channel->getRemoteEndpoint().getAddress();
    auto const port    = channel->getRemoteEndpoint().getPort();
    return ((_ip == address) && (_port == port));
}

} // namespace internal

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
