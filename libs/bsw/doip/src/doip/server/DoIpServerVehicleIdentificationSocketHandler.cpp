// Copyright 2025 Accenture.

#include "doip/server/DoIpServerVehicleIdentificationSocketHandler.h"

#include "bsp/timer/SystemTimer.h"
#include "doip/common/DoIpConstants.h"
#include "doip/common/DoIpLock.h"
#include "doip/common/IDoIpVehicleAnnouncementListener.h"
#include "doip/server/DoIpServerVehicleIdentificationConfig.h"
#include "doip/server/IDoIpServerEntityStatusCallback.h"
#include "doip/server/IDoIpServerVehicleAnnouncementParameterProvider.h"
#include "doip/server/IDoIpServerVehicleIdentificationCallback.h"

#include <estd/array.h>
#include <estd/big_endian.h>
#include <estd/memory.h>
#include <estd/optional.h>

namespace doip
{
DoIpServerVehicleIdentificationSocketHandler::DoIpServerVehicleIdentificationSocketHandler(
    DoIpConstants::ProtocolVersion const protocolVersion,
    ::udp::AbstractDatagramSocket& socket,
    uint8_t const socketGroupId,
    ::ip::NetworkInterfaceConfigKey const& networkInterfaceConfigKey,
    ::ip::IPAddress const& multicastAddress,
    ::estd::vector<::ip::IPAddress>& unicastAddresses,
    DoIpServerVehicleIdentificationConfig& config,
    uint8_t const announceCount)
: IDoIpConnectionHandler()
, _connection(config.getContext(), socket, config.getWriteBuffer())
, _multicastAddress(multicastAddress)
, _config(config)
, _vehicleAnnouncementListener(nullptr)
, _enqueueInitialBroadcastsUnicastAsync(
      ::async::Function::CallType::create<
          DoIpServerVehicleIdentificationSocketHandler,
          &DoIpServerVehicleIdentificationSocketHandler::enqueueInitialBroadcastsUnicastAsync>(
          *this))
, _configChangedContinuationAsync(
      ::async::Function::CallType::create<
          DoIpServerVehicleIdentificationSocketHandler,
          &DoIpServerVehicleIdentificationSocketHandler::configChangedContinuationAsync>(*this))
, _configChangedSlot(
      ::ip::NetworkInterfaceConfigRegistry::ConfigChangedSignal::slot_function::create<
          DoIpServerVehicleIdentificationSocketHandler,
          &DoIpServerVehicleIdentificationSocketHandler::configChanged>(*this))
, _unicastAddresses(unicastAddresses)
, _sendJob()
, _protocolVersion(protocolVersion)
, _networkInterfaceConfigKey(networkInterfaceConfigKey)
, _configChangedNewConfig{}
, _socketGroupId(socketGroupId)
, _announceCount(announceCount)
{}

void DoIpServerVehicleIdentificationSocketHandler::updateUnicastAddresses(
    ::ip::NetworkInterfaceConfigKey const& configKey,
    ::estd::slice<ip::IPAddress const> const unicastAddresses)
{
    if (configKey == _networkInterfaceConfigKey)
    {
        {
            // RAII mutex
            DoIpLock const lock;
            if (_unicastAddresses.max_size() < unicastAddresses.size())
            {
                return;
            }
            (void)_newUnicastAddresses.reset();
            ::estd::slice<::ip::IPAddress const> const prevAddresses = getUnicastAddresses();
            for (size_t idx = 0U; idx < unicastAddresses.size(); ++idx)
            {
                (void)_newUnicastAddresses.set(
                    idx,
                    std::find(prevAddresses.begin(), prevAddresses.end(), unicastAddresses[idx])
                        == prevAddresses.end());
            }

            _unicastAddresses.clear();
            _unicastAddresses.insert(
                _unicastAddresses.end(), unicastAddresses.begin(), unicastAddresses.end());
        }
        ::async::execute(_config.getContext(), _enqueueInitialBroadcastsUnicastAsync);
    }
}

::estd::slice<ip::IPAddress const>
DoIpServerVehicleIdentificationSocketHandler::getUnicastAddresses() const
{
    return {_unicastAddresses};
}

void DoIpServerVehicleIdentificationSocketHandler::sendAnnouncement()
{
    enqueueAny(
        DoIpServerVehicleIdentificationRequest::ISOType::IDENTIFICATION,
        0U,
        ::ip::IPEndpoint(_multicastAddress, DoIpConstants::Ports::UDP_DISCOVERY),
        _config.getParameters().getAnnounceWait());
}

void DoIpServerVehicleIdentificationSocketHandler::start()
{
    _config.getNetworkInterfaceConfigRegistry().configChangedSignal.connect(_configChangedSlot);
    configChanged(
        _networkInterfaceConfigKey,
        _config.getNetworkInterfaceConfigRegistry().getConfig(_networkInterfaceConfigKey));
}

void DoIpServerVehicleIdentificationSocketHandler::shutdown()
{
    _timeoutTimeout.cancel();
    _config.getNetworkInterfaceConfigRegistry().configChangedSignal.disconnect(_configChangedSlot);
    configChanged(_networkInterfaceConfigKey, ::ip::NetworkInterfaceConfig());
}

// closed connection is not handled on purpose
void DoIpServerVehicleIdentificationSocketHandler::connectionClosed(bool const closedByRemotePeer)
{
    (void)closedByRemotePeer;
}

IDoIpConnectionHandler::HeaderReceivedContinuation
DoIpServerVehicleIdentificationSocketHandler::headerReceived(DoIpHeader const& header)
{
    if (checkVersion(header))
    {
        return handleRequest(header) ? IDoIpConnectionHandler::
                       HeaderReceivedContinuation{IDoIpConnectionHandler::HandledByThisHandler{}}
                                     : IDoIpConnectionHandler::HeaderReceivedContinuation{
                                         IDoIpConnection::PayloadDiscardedCallbackType{}};
    }
    _connection.endReceiveMessage(IDoIpConnection::PayloadDiscardedCallbackType{});
    return IDoIpConnectionHandler::HeaderReceivedContinuation{
        IDoIpConnectionHandler::HandledByThisHandler{}};
}

void DoIpServerVehicleIdentificationSocketHandler::enqueueInitialBroadcastsUnicastAsync()
{
    for (size_t idx = 0U; idx < _unicastAddresses.size(); ++idx)
    {
        bool isNew = false;
        {
            // RAII lock
            DoIpLock const lock;
            isNew = _newUnicastAddresses.test(idx);
            _newUnicastAddresses.reset(idx);
        }
        if (isNew)
        {
            enqueueInitialBroadcasts(
                ::ip::IPEndpoint(_unicastAddresses[idx], DoIpConstants::Ports::UDP_DISCOVERY));
        }
    }
}

void DoIpServerVehicleIdentificationSocketHandler::configChangedContinuationAsync()
{
    // clear pending requests
    _pendingRequests.clear();
    _connection.close();

    bool isValid;
    ::ip::IPAddress bindAddress;
    ::ip::IPAddress broadcastAddress;
    {
        // RAII lock
        DoIpLock const lock;
        isValid          = _configChangedNewConfig.isValid();
        bindAddress      = _configChangedNewConfig.ipAddress();
        broadcastAddress = _configChangedNewConfig.broadcastAddress();
    }
    if (!isValid)
    {
        return;
    }

    _connection.init(*this);
    (void)_connection.getSocket().bind(&bindAddress, DoIpConstants::Ports::UDP_DISCOVERY);
    if (_configChangedNewConfig.ipFamily() == ::ip::IPAddress::IPV4)
    {
        if (_multicastAddress != ::ip::make_ip4(255U, 255U, 255U, 255U))
        {
            // leave local broadcast in place if set by the user
            _multicastAddress = broadcastAddress;
        }
    }
    else
    {
        (void)_connection.getSocket().join(_multicastAddress);
    }
    _unicastAddresses.clear();
    // send initial announcements
    enqueueInitialBroadcasts(
        ::ip::IPEndpoint(_multicastAddress, DoIpConstants::Ports::UDP_DISCOVERY));
}

void DoIpServerVehicleIdentificationSocketHandler::execute() { trySendResponse(); }

void DoIpServerVehicleIdentificationSocketHandler::configChanged(
    ::ip::NetworkInterfaceConfigKey const key, ::ip::NetworkInterfaceConfig const& config)
{
    if (key == _networkInterfaceConfigKey)
    {
        // cancel timeout to stop any announce cycles currently in progress
        _timeoutTimeout.cancel();
        {
            // RAII lock
            DoIpLock const lock;
            _configChangedNewConfig = config;
        }
        ::async::execute(_config.getContext(), _configChangedContinuationAsync);
    }
}

bool DoIpServerVehicleIdentificationSocketHandler::checkVersion(DoIpHeader const& header)
{
    uint16_t const payloadType = header.payloadType;
    if (payloadType == DoIpConstants::PayloadTypes::NEGATIVE_ACK)
    {
        return false;
    }

    if (checkProtocolVersion(header, static_cast<uint8_t>(_protocolVersion)))
    {
        return true;
    }

    if (checkProtocolVersion(header, 0xffU))
    {
        if ((payloadType == DoIpConstants::PayloadTypes::VEHICLE_IDENTIFICATION_REQUEST_MESSAGE)
            || (payloadType
                == DoIpConstants::PayloadTypes::VEHICLE_IDENTIFICATION_REQUEST_MESSAGE_EID)
            || (payloadType
                == DoIpConstants::PayloadTypes::VEHICLE_IDENTIFICATION_REQUEST_MESSAGE_VIN))
        {
            return true;
        }

        enqueueNack(DoIpConstants::NackCodes::NACK_UNKNOWN_PAYLOAD_TYPE);
        return false;
    }

    enqueueNack(DoIpConstants::NackCodes::NACK_INCORRECT_PATTERN);
    return false;
}

bool DoIpServerVehicleIdentificationSocketHandler::handleVehicleIdentificationRequestMessage(
    DoIpHeader const& header)
{
    if (header.payloadLength == 0U)
    {
        enqueueResponse(DoIpServerVehicleIdentificationRequest::ISOType::IDENTIFICATION);
        return true;
    }
    enqueueNack(DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH);
    return false;
}

bool DoIpServerVehicleIdentificationSocketHandler::handleVehicleIdentificationRequestMessageEid(
    DoIpHeader const& header)
{
    if (header.payloadLength == 6U)
    {
        return _connection.receivePayload(
            ::estd::slice<uint8_t>(_readBuffer).trim(6U),
            IDoIpConnection::PayloadReceivedCallbackType::create<
                DoIpServerVehicleIdentificationSocketHandler,
                &DoIpServerVehicleIdentificationSocketHandler::
                    vehicleIdentificationEidRequestReceived>(*this));
    }
    enqueueNack(DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH);
    return false;
}

bool DoIpServerVehicleIdentificationSocketHandler::handleVehicleIdentificationRequestMessageVin(
    DoIpHeader const& header)
{
    if (header.payloadLength == 17U)
    {
        return _connection.receivePayload(
            ::estd::make_slice(_readBuffer).trim(17U),
            IDoIpConnection::PayloadReceivedCallbackType::create<
                DoIpServerVehicleIdentificationSocketHandler,
                &DoIpServerVehicleIdentificationSocketHandler::
                    vehicleIdentificationVinRequestReceived>(*this));
    }
    enqueueNack(DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH);
    return false;
}

bool DoIpServerVehicleIdentificationSocketHandler::handleEntityStatusRequest(
    DoIpHeader const& header)
{
    if (header.payloadLength == 0U)
    {
        enqueueResponse(DoIpServerVehicleIdentificationRequest::ISOType::ENTITYSTATUS);
        return true;
    }
    enqueueNack(DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH);
    return false;
}

bool DoIpServerVehicleIdentificationSocketHandler::handleDiagnosticPowerModeInformationRequest(
    DoIpHeader const& header)
{
    if (header.payloadLength == 0U)
    {
        enqueueResponse(DoIpServerVehicleIdentificationRequest::ISOType::DIAGNOSTICPOWERMODEINFO);
        return true;
    }
    enqueueNack(DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH);
    return false;
}

bool DoIpServerVehicleIdentificationSocketHandler::handleVehicleAnnouncementMessage(
    DoIpHeader const& header)
{
    if (_vehicleAnnouncementListener == nullptr)
    {
        // discard vehicle announcement without sending a negative response
        return false;
    }
    if ((header.payloadLength >= 32U) && (header.payloadLength <= 33U))
    {
        return _connection.receivePayload(
            ::estd::make_slice(_readBuffer).trim(19U),
            IDoIpConnection::PayloadReceivedCallbackType::create<
                DoIpServerVehicleIdentificationSocketHandler,
                &DoIpServerVehicleIdentificationSocketHandler::vehicleAnnouncementPayloadReceived>(
                *this));
    }
    enqueueNack(DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH);
    return false;
}

bool DoIpServerVehicleIdentificationSocketHandler::handleOemRequest(DoIpHeader const& header)
{
    uint16_t const payloadType = static_cast<uint16_t>(header.payloadType);
    if (!DoIpConstants::isValidOemRequest(payloadType))
    {
        return false;
    }
    IDoIpUdpOemMessageHandler const* const oemMessageHandler
        = _config.getVehicleIdentificationCallback().getOemMessageHandler(payloadType);
    if (oemMessageHandler == nullptr)
    {
        return false;
    }
    if (header.payloadLength == oemMessageHandler->getRequestPayloadSize())
    {
        if (header.payloadLength == 0U)
        {
            enqueueResponse(oemMessageHandler->getRequestPayloadType());
        }
        else
        {
            // save the message handler; the payload bytes will be received sequentially after the
            // header; we can use the saved message handler to parse the payload
            _currentOemMessageHandler = oemMessageHandler;
            // receive payload
            return _connection.receivePayload(
                ::estd::slice<uint8_t>(_readBuffer).trim(header.payloadLength),
                IDoIpConnection::PayloadReceivedCallbackType::create<
                    DoIpServerVehicleIdentificationSocketHandler,
                    &DoIpServerVehicleIdentificationSocketHandler::oemMessagePayloadReceived>(
                    *this));
        }
    }
    else
    {
        enqueueNack(DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH);
    }
    return true;
}

bool DoIpServerVehicleIdentificationSocketHandler::handleRequest(DoIpHeader const& header)
{
    switch (static_cast<uint16_t>(header.payloadType))
    {
        case DoIpConstants::PayloadTypes::VEHICLE_IDENTIFICATION_REQUEST_MESSAGE:
        {
            return handleVehicleIdentificationRequestMessage(header);
        }
        case DoIpConstants::PayloadTypes::VEHICLE_IDENTIFICATION_REQUEST_MESSAGE_EID:
        {
            return handleVehicleIdentificationRequestMessageEid(header);
        }
        case DoIpConstants::PayloadTypes::VEHICLE_IDENTIFICATION_REQUEST_MESSAGE_VIN:
        {
            return handleVehicleIdentificationRequestMessageVin(header);
        }
        case DoIpConstants::PayloadTypes::ENTITY_STATUS_REQUEST:
        {
            return handleEntityStatusRequest(header);
        }
        case DoIpConstants::PayloadTypes::DIAGNOSTIC_POWER_MODE_INFORMATION_REQUEST:
        {
            return handleDiagnosticPowerModeInformationRequest(header);
        }
        case DoIpConstants::PayloadTypes::VEHICLE_ANNOUNCEMENT_MESSAGE:
        {
            return handleVehicleAnnouncementMessage(header);
        }
        default:
        {
            if (handleOemRequest(header))
            {
                return true;
            }
            enqueueNack(DoIpConstants::NackCodes::NACK_UNKNOWN_PAYLOAD_TYPE);
            break;
        }
    }
    return false;
}

void DoIpServerVehicleIdentificationSocketHandler::vehicleIdentificationEidRequestReceived(
    ::estd::slice<uint8_t const> const payload)
{
    uint8_t eid[6];
    _config.getVehicleIdentificationCallback().getEid(eid);
    if (::estd::memory::is_equal(payload, eid))
    {
        enqueueResponse(DoIpServerVehicleIdentificationRequest::ISOType::IDENTIFICATION);
    }
    _connection.endReceiveMessage(IDoIpConnection::PayloadDiscardedCallbackType{});
}

void DoIpServerVehicleIdentificationSocketHandler::vehicleIdentificationVinRequestReceived(
    ::estd::slice<uint8_t const> const payload)
{
    uint8_t vin[17];
    _config.getVehicleIdentificationCallback().getVin(
        ::estd::make_static_slice(vin).reinterpret_as<char>());
    if (::estd::memory::is_equal(payload, vin))
    {
        enqueueResponse(DoIpServerVehicleIdentificationRequest::ISOType::IDENTIFICATION);
    }
    _connection.endReceiveMessage(IDoIpConnection::PayloadDiscardedCallbackType{});
}

void DoIpServerVehicleIdentificationSocketHandler::vehicleAnnouncementPayloadReceived(
    ::estd::slice<uint8_t const> const payload)
{
    if (_vehicleAnnouncementListener != nullptr)
    {
        uint16_t const logicalAddress = ::estd::read_be<uint16_t>(&payload[17]);
        _vehicleAnnouncementListener->vehicleAnnouncementReceived(
            logicalAddress, _connection.getLocalEndpoint(), _connection.getRemoteEndpoint());
    }
    _connection.endReceiveMessage(IDoIpConnection::PayloadDiscardedCallbackType{});
}

void DoIpServerVehicleIdentificationSocketHandler::oemMessagePayloadReceived(
    ::estd::slice<uint8_t const> const payload)
{
    if (_currentOemMessageHandler != nullptr)
    {
        uint8_t nackCode = 0x00U;
        if (_currentOemMessageHandler->onPayloadReceived(payload, nackCode))
        {
            enqueueResponse(_currentOemMessageHandler->getRequestPayloadType());
        }
        else
        {
            enqueueNack(nackCode);
        }
        _currentOemMessageHandler = nullptr;
    }
    _connection.endReceiveMessage(IDoIpConnection::PayloadDiscardedCallbackType{});
}

void DoIpServerVehicleIdentificationSocketHandler::enqueueResponse(
    DoIpServerVehicleIdentificationRequest::Type const& type)
{
    bool const needsDelay = type == DoIpServerVehicleIdentificationRequest::ISOType::IDENTIFICATION;
    enqueueAny(
        type,
        0U,
        _connection.getRemoteEndpoint(),
        needsDelay ? _config.getParameters().getAnnounceWait() : 0U);
}

void DoIpServerVehicleIdentificationSocketHandler::enqueueNack(uint8_t const nackCode)
{
    enqueueAny(
        DoIpServerVehicleIdentificationRequest::ISOType::NACK,
        nackCode,
        _connection.getRemoteEndpoint(),
        0U);
}

void DoIpServerVehicleIdentificationSocketHandler::enqueueAny(
    DoIpServerVehicleIdentificationRequest::Type const& type,
    uint8_t const nackCode,
    ::ip::IPEndpoint const& endpoint,
    uint16_t const timeoutInMs)
{
    if (!_config.getRequestPool().empty())
    {
        // keep list sorted by inserting via merge
        uint32_t const systemTime = getSystemTimeMs32Bit();
        ::estd::forward_list<DoIpServerVehicleIdentificationRequest> tmp;
        tmp.push_front(_config.getRequestPool().allocate().construct(
            endpoint, type, nackCode, timeoutInMs + systemTime));
        _pendingRequests.merge(
            tmp,
            [](DoIpServerVehicleIdentificationRequest const& left,
               DoIpServerVehicleIdentificationRequest const& right) -> bool
            { return left.getScheduledTime() < right.getScheduledTime(); });

        // now schedule the function
        scheduleMessage();
    }
}

void DoIpServerVehicleIdentificationSocketHandler::enqueueInitialBroadcasts(
    ::ip::IPEndpoint const& endpoint)
{
    uint16_t const announceWait     = _config.getParameters().getAnnounceWait();
    uint16_t const announceInterval = _config.getParameters().getAnnounceInterval();
    for (uint8_t i = 0; i < _announceCount; i++)
    {
        uint16_t const relativeTimeout = announceWait + i * announceInterval;
        enqueueAny(
            DoIpServerVehicleIdentificationRequest::ISOType::IDENTIFICATION,
            0U,
            endpoint,
            relativeTimeout);
    }
}

void DoIpServerVehicleIdentificationSocketHandler::scheduleMessage()
{
    uint32_t const currTimestamp = getSystemTimeMs32Bit();
    uint32_t const delay         = _pendingRequests.front().getScheduledTime() > currTimestamp
                                       ? _pendingRequests.front().getScheduledTime() - currTimestamp
                                       : 0U;
    // cancel timeout (if any)
    _timeoutTimeout.cancel();
    (void)::async::schedule(
        _config.getContext(), *this, _timeoutTimeout, delay, ::async::TimeUnit::MILLISECONDS);
}

void DoIpServerVehicleIdentificationSocketHandler::trySendResponse()
{
    {
        // RAII lock not unused
        // protect from release callback
        DoIpLock const lock;
        if (_sendJob.has_value())
        {
            return;
        }
    }
    if (!_pendingRequests.empty())
    {
        DoIpServerVehicleIdentificationRequest& request = _pendingRequests.front();
        _pendingRequests.pop_front();
        sendResponse(request.getDestinationEndpoint(), request.getType(), request.getNackCode());
        _config.getRequestPool().release(request);
        return;
    }
}

DoIpServerVehicleIdentificationSocketHandler::StaticPayloadSendJobType&
DoIpServerVehicleIdentificationSocketHandler::createResponseIdentification()
{
    auto& sendJob = allocateSendJob(DoIpConstants::PayloadTypes::VEHICLE_ANNOUNCEMENT_MESSAGE, 32U);
    ::estd::slice<uint8_t> payloadBuffer = sendJob.accessPayloadBuffer();
    // cast uint8_t* -> char* is safe
    _config.getVehicleIdentificationCallback().getVin(
        IDoIpServerVehicleIdentificationCallback::VinType::from_pointer(
            reinterpret_cast<char*>(payloadBuffer.data())));
    payloadBuffer.advance(17U);
    ::estd::memory::take<::estd::be_uint16_t>(payloadBuffer) = _config.getLogicalEntityAddress();
    _config.getVehicleIdentificationCallback().getEid(
        IDoIpServerVehicleIdentificationCallback::EidType::from_pointer(payloadBuffer.data()));
    payloadBuffer.advance(6U);
    _config.getVehicleIdentificationCallback().getGid(
        IDoIpServerVehicleIdentificationCallback::GidType::from_pointer(payloadBuffer.data()));
    payloadBuffer[6] = 0x00U;
    return sendJob;
}

DoIpServerVehicleIdentificationSocketHandler::StaticPayloadSendJobType&
DoIpServerVehicleIdentificationSocketHandler::createResponseEntityStatus()
{
    auto& sendJob = allocateSendJob(DoIpConstants::PayloadTypes::ENTITY_STATUS_RESPONSE, 7U);
    IDoIpServerEntityStatusCallback::EntityStatus const entityStatus
        = _config.getEntityStatusCallback().getEntityStatus(_socketGroupId);

    ::estd::slice<uint8_t> const writeBuffer = sendJob.accessPayloadBuffer();
    writeBuffer[0]                           = entityStatus._nodeType;
    writeBuffer[1]                           = entityStatus._maxConnectionCount;
    writeBuffer[2]                           = entityStatus._connectionCount;
    ::estd::write_be<uint32_t>(&writeBuffer[3], entityStatus._maxDataSize);
    return sendJob;
}

DoIpServerVehicleIdentificationSocketHandler::StaticPayloadSendJobType&
DoIpServerVehicleIdentificationSocketHandler::createResponseDiagnosticPowerModeInfo()
{
    auto& sendJob = allocateSendJob(
        DoIpConstants::PayloadTypes::DIAGNOSTIC_POWER_MODE_INFORMATION_RESPONSE, 1U);
    sendJob.accessPayloadBuffer()[0U]
        = static_cast<uint8_t>(_config.getVehicleIdentificationCallback().getPowerMode());
    return sendJob;
}

// pointer checked beforehand
DoIpServerVehicleIdentificationSocketHandler::StaticPayloadSendJobType&
DoIpServerVehicleIdentificationSocketHandler::createResponseOemMessage(
    IDoIpUdpOemMessageHandler const* const oemMessageHandler)
{
    uint16_t const responsePayloadType = oemMessageHandler->getResponsePayloadType();
    uint8_t const responsePayloadSize  = oemMessageHandler->getResponsePayloadSize();
    auto& sendJob                      = allocateSendJob(responsePayloadType, responsePayloadSize);
    oemMessageHandler->createResponse(sendJob.accessPayloadBuffer());
    return sendJob;
}

DoIpServerVehicleIdentificationSocketHandler::StaticPayloadSendJobType&
DoIpServerVehicleIdentificationSocketHandler::createResponseHeaderNack(uint8_t const nackCode)
{
    auto& sendJob = allocateSendJob(DoIpConstants::PayloadTypes::NEGATIVE_ACK, 1U);
    sendJob.accessPayloadBuffer()[0U] = nackCode;
    return sendJob;
}

DoIpServerVehicleIdentificationSocketHandler::StaticPayloadSendJobType&
DoIpServerVehicleIdentificationSocketHandler::createResponse(
    DoIpServerVehicleIdentificationRequest::Type const& type, uint8_t const nackCode)
{
    if (type.is<DoIpServerVehicleIdentificationRequest::ISOType>())
    {
        switch (type.get<DoIpServerVehicleIdentificationRequest::ISOType>())
        {
            case DoIpServerVehicleIdentificationRequest::ISOType::IDENTIFICATION:
            {
                return createResponseIdentification();
            }
            case DoIpServerVehicleIdentificationRequest::ISOType::ENTITYSTATUS:
            {
                return createResponseEntityStatus();
            }
            case DoIpServerVehicleIdentificationRequest::ISOType::DIAGNOSTICPOWERMODEINFO:
            {
                return createResponseDiagnosticPowerModeInfo();
            }
            default:
            {
                return createResponseHeaderNack(nackCode);
            }
        }
    }
    else
    {
        uint16_t const oemPayloadType = type.get<uint16_t>();
        IDoIpUdpOemMessageHandler const* const oemMessageHandler
            = _config.getVehicleIdentificationCallback().getOemMessageHandler(oemPayloadType);
        if (oemMessageHandler != nullptr)
        {
            return createResponseOemMessage(oemMessageHandler);
        }

        // should never be reached
        return createResponseHeaderNack(nackCode);
    }
}

void DoIpServerVehicleIdentificationSocketHandler::sendResponse(
    ::ip::IPEndpoint const& destinationEndpoint,
    DoIpServerVehicleIdentificationRequest::Type const& type,
    uint8_t const nackCode)
{
    auto& sendJob        = createResponse(type, nackCode);
    _destinationEndpoint = destinationEndpoint;
    sendJob.setDestinationEndpoint(&_destinationEndpoint);
    if (!_connection.sendMessage(sendJob))
    {
        releaseSendJob();
    }
}

DoIpServerVehicleIdentificationSocketHandler::StaticPayloadSendJobType&
DoIpServerVehicleIdentificationSocketHandler::allocateSendJob(
    uint16_t const payloadType, uint8_t const payloadLength)
{
    estd_assert(!_sendJob.has_value());
    return _sendJob.emplace().construct(
        static_cast<uint8_t>(_protocolVersion),
        payloadType,
        payloadLength,
        StaticPayloadSendJobType::ReleaseCallbackType::create<
            DoIpServerVehicleIdentificationSocketHandler,
            &DoIpServerVehicleIdentificationSocketHandler::releaseSendJobAndSendNext>(*this));
}

void DoIpServerVehicleIdentificationSocketHandler::releaseSendJob()
{
    {
        // RAII lock not unused
        DoIpLock const lock;
        _sendJob.reset();
    }
}

void DoIpServerVehicleIdentificationSocketHandler::releaseSendJobAndSendNext(
    IDoIpSendJob& /*sendJob*/, bool const /*success*/)
{
    releaseSendJob();
    if (!_pendingRequests.empty())
    {
        scheduleMessage();
    }
}

} // namespace doip
