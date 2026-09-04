/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SdMessageParser.h"

#include "someip/SdConstants.h"
#include "someip/SdOptionParser.h"
#include "someip/SomeIpConstants.h"
#include "someip/SomeIpMessage.h"
#include "someip/Statistics.h"
#include "someip/logger.h"

#include <ip/IPAddress.h>

#include "someip/endian_helpers.h"

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::util::logger::SOMEIP;

namespace
{
SdEndpoint searchIPMulticastOption(SdOptions const& options)
{
    uint8_t numUdpOptions = 0U;
    uint8_t numTcpOptions = 0U;

    SdEndpoint const endpoint
        = SdOptionParser::parseIpMulticastOption(options, numUdpOptions, numTcpOptions);

    if (!endpoint.isValid())
    {
        return SdOptionParser::INVALID_ENDPOINT;
    }

    if ((numUdpOptions > 1U) || (numTcpOptions > 1U))
    {
        return SdOptionParser::INVALID_ENDPOINT;
    }

    if (!::ip::isMulticastAddress(endpoint.getAddress()))
    {
        return SdOptionParser::INVALID_ENDPOINT;
    }

    return endpoint;
}

SdEndpoint searchIPEndpointOption(
    SdOptions const& options, ::ip::IPAddress const& localIp, uint8_t const subnetId)
{
    uint8_t numUdpOptions = 0U;
    uint8_t numTcpOptions = 0U;

    SdEndpoint const endpoint
        = SdOptionParser::parseIpEndpointOption(options, numUdpOptions, numTcpOptions);

    if (!endpoint.isValid())
    {
        return SdOptionParser::INVALID_ENDPOINT;
    }

    if ((numUdpOptions > 1U) || (numTcpOptions > 1U))
    {
        return SdOptionParser::INVALID_ENDPOINT;
    }

    if ((!::ip::isNetworkLocal(endpoint.getAddress(), localIp, subnetId))
        || (endpoint.getAddress() == localIp))
    {
        return SdOptionParser::INVALID_ENDPOINT;
    }

    return endpoint;
}

} // namespace

SdMessageParser::SdMessageParser(
    IServiceRegistry& serviceRegistry,
    IServiceAnnouncer& serviceAnnouncer,
    RebootTracker& rebootTracker,
    uint8_t const subnetId,
    ::ip::IPAddress const& localIp,
    AdditionalSDCheck const additionalSDCheck)
: _serviceRegistry(serviceRegistry)
, _serviceAnnouncer(serviceAnnouncer)
, _subnetId(subnetId)
, _localIp(localIp)
, _rebootTracker(rebootTracker)
, _additionalSDCheck(additionalSDCheck)
{}

void SdMessageParser::init() { _rebootTracker.init(); }

// virtual
void SdMessageParser::handleMessage(
    SomeIpMessage const& message, ::ip::IPEndpoint const& sourceEndpoint, bool const isMulticast)
{
    if (message.getProtocolVersion() != configuration::PROTOCOL_VERSION)
    {
        WARN_LOG(
            SOMEIP,
            "SdMessageParser::handleMessage() invalid protocol: %d",
            message.getProtocolVersion());
        Statistics::incCounter(Statistics::Counter::SD_MALFORMED_MESSAGE_RX);
        return;
    }

    if (message.getInterfaceVersion() != configuration::INTERFACE_VERSION)
    {
        WARN_LOG(
            SOMEIP,
            "SdMessageParser::handleMessage() invalid interface: %d",
            message.getInterfaceVersion());
        Statistics::incCounter(Statistics::Counter::SD_MALFORMED_MESSAGE_RX);
        return;
    }

    if (message.getMessageType() != SomeIpMessage::MessageType::NOTIFICATION)
    {
        WARN_LOG(
            SOMEIP,
            "SdMessageParser::handleMessage() invalid message type: %d",
            message.getMessageType());
        Statistics::incCounter(Statistics::Counter::SD_MALFORMED_MESSAGE_RX);
        return;
    }

    if (message.getClientId() != 0U)
    {
        WARN_LOG(
            SOMEIP, "SdMessageParser::handleMessage() invalid client: %d", message.getClientId());
        Statistics::incCounter(Statistics::Counter::SD_MALFORMED_MESSAGE_RX);
        return;
    }

    uint16_t const sessionId = static_cast<uint16_t>(message.getSessionId());
    bool const sdFlagReboot
        = ((message.getFlags() & static_cast<uint8_t>(SdFlags::SD_FLAG_REBOOT)) != 0U);

    SessionInfo const session
        = SessionInfo(sourceEndpoint.getAddress(), isMulticast, sessionId, sdFlagReboot);

    if (_rebootTracker.evaluate(session))
    {
        Statistics::incCounter(Statistics::Counter::SD_REBOOT);
        _serviceRegistry.rebootDetected(sourceEndpoint.getAddress());
    }

    if (parseMessage(message.getBufferPayload(), sourceEndpoint, isMulticast))
    {
        _rebootTracker.apply(session);
    }
    else
    {
        Statistics::incCounter(Statistics::Counter::SD_MALFORMED_MESSAGE_RX);
    }
}

// private
bool SdMessageParser::parseMessage(
    ::etl::span<uint8_t const> const& payload,
    ::ip::IPEndpoint const& sourceEndpoint,
    bool const receivedByMulticast)
{
    size_t offset = 0U;

    if (payload.size() < static_cast<uint16_t>(SdConstants::SD_ENTRIES_LENGTH_OFFSET)
                             + static_cast<uint16_t>(SdConstants::SD_ENTRIES_LENGTH_FIELD_LENGTH))
    {
        return false;
    }

    uint8_t const flags      = payload[offset];
    bool const sdFlagUnicast = ((flags & static_cast<uint8_t>(SdFlags::SD_FLAG_UNICAST)) != 0U);
    offset += static_cast<uint16_t>(SdConstants::SD_ENTRIES_LENGTH_OFFSET);

    uint32_t const entriesLength = ::someip::endian::read_be<uint32_t>(&payload[offset]);
    if (payload.size() < (offset + static_cast<size_t>(entriesLength)))
    {
        return false;
    }
    offset += static_cast<uint16_t>(SdConstants::SD_ENTRIES_LENGTH_FIELD_LENGTH);
    uint32_t const entriesOffset = static_cast<uint32_t>(offset);

    SdOptions options;

    if (options.init(payload, entriesLength) == false)
    {
        return false;
    }

    bool containsRelevantService = false;
    while ((((offset + static_cast<uint16_t>(SdConstants::SD_ENTRY_LENGTH)) - entriesOffset)
            <= entriesLength)
           && ((offset + static_cast<uint16_t>(SdConstants::SD_ENTRY_LENGTH)) <= payload.size()))
    {
        bool const parseEntryResult = parseEntry(
            payload.subspan(offset), sourceEndpoint, receivedByMulticast, sdFlagUnicast, options);

        containsRelevantService = (parseEntryResult || containsRelevantService);
        offset += static_cast<uint16_t>(SdConstants::SD_ENTRY_LENGTH);
    }

    return containsRelevantService;
}

// private
bool SdMessageParser::parseEntry(
    ::etl::span<uint8_t const> const& entry,
    ::ip::IPEndpoint const& sourceEndpoint,
    bool const receivedByMulticast,
    bool const sdFlagUnicast,
    SdOptions& options)
{
    uint8_t const entryType = entry[static_cast<uint16_t>(SdConstants::SD_ENTRY_TYPE_OFFSET)];
    service_id::type const serviceId = ::someip::endian::read_be<uint16_t>(
        &entry[static_cast<uint16_t>(SdConstants::SD_SERVICE_ID_OFFSET)]);
    instance_id::type const instanceId = ::someip::endian::read_be<uint16_t>(
        &entry[static_cast<uint16_t>(SdConstants::SD_INSTANCE_ID_OFFSET)]);
    major_version::type const majorVersion
        = entry[static_cast<uint16_t>(SdConstants::SD_MAJOR_VERSION_OFFSET)];
    ttl::type const ttl
        = ::someip::endian::read_be_24(&entry[static_cast<uint16_t>(SdConstants::SD_TTL_OFFSET)]);
    minor_version::type const minorVersion = ::someip::endian::read_be<uint32_t>(
        &entry[static_cast<uint16_t>(SdConstants::SD_MINOR_VERSION_OFFSET)]);
    uint16_t const reserved = ::someip::endian::read_be<uint16_t>(
        &entry[static_cast<uint16_t>(SdConstants::SD_RESERVED_OFFSET)]);
    eventgroup_id::type const eventgroup = ::someip::endian::read_be<uint16_t>(
        &entry[static_cast<uint16_t>(SdConstants::SD_EVENTGROUP_OFFSET)]);

    // A SUBSCRIBE might contain invalid service or event group which we have to reject with Nack!
    if ((ENTRY_TYPE_SUBSCRIBE != entryType)
        && (!_serviceRegistry.interestedInService(serviceId, instanceId, majorVersion)))
    {
        return false;
    }

    switch (entryType)
    {
        case ENTRY_TYPE_FIND:
        {
            Statistics::incCounter(Statistics::Counter::SD_FIND_RX);
            handleEntryFind(
                serviceId,
                instanceId,
                majorVersion,
                ttl,
                minorVersion,
                sourceEndpoint.getAddress(),
                sdFlagUnicast);
            break;
        }
        case ENTRY_TYPE_OFFER:
        {
            Statistics::incCounter(Statistics::Counter::SD_OFFER_RX);
            options.readIndexValues(entry);
            handleEntryOffer(
                options, serviceId, instanceId, majorVersion, ttl, minorVersion, sourceEndpoint);
            break;
        }
        case ENTRY_TYPE_FIND_EVENTGROUP:
        {
            Statistics::incCounter(Statistics::Counter::SD_FIND_EVENTGROUP_RX);
            DEBUG_LOG(SOMEIP, "SdMessageParser::parseEntry() unsupported type: %d", entryType);
            break;
        }
        case ENTRY_TYPE_PUBLISH:
        {
            Statistics::incCounter(Statistics::Counter::SD_PUBLISH_RX);
            DEBUG_LOG(SOMEIP, "SdMessageParser::parseEntry() unsupported type: %d", entryType);
            break;
        }
        case ENTRY_TYPE_SUBSCRIBE:
        {
            Statistics::incCounter(Statistics::Counter::SD_SUBSCRIBE_RX);
            if (receivedByMulticast)
            {
                WARN_LOG(
                    SOMEIP,
                    "SdMessageParser::parseEntry() discard subscribe received by multicast");
                break; // SIP_SD_818 : discard
            }
            options.readIndexValues(entry);
            handleEntrySubscribe(
                options,
                serviceId,
                instanceId,
                majorVersion,
                eventgroup,
                ttl,
                reserved,
                sourceEndpoint);
            break;
        }
        case ENTRY_TYPE_SUBSCRIBE_ACK:
        {
            if (receivedByMulticast)
            {
                break; // SIP_SD_818 : discard
            }
            options.readIndexValues(entry);
            handleEntrySubscribeAck(
                options,
                serviceId,
                instanceId,
                majorVersion,
                eventgroup,
                ttl,
                sourceEndpoint.getAddress());
            break;
        }
        default:
        {
            Statistics::incCounter(Statistics::Counter::SD_UNKNOWN_RX);
            WARN_LOG(SOMEIP, "SdMessageParser::parseEntry() invalid type: %d", entryType);
            break;
        }
    }

    return true;
}

// private
void SdMessageParser::handleEntryFind(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    major_version::type const majorVersion,
    ttl::type const ttl,
    minor_version::type const minorVersion,
    ::ip::IPAddress const& sourceAddress,
    bool const sdFlagUnicast)
{
    if (ttl != 0U)
    {
        _serviceAnnouncer.respondToFindService(
            serviceId, instanceId, majorVersion, minorVersion, ttl, sourceAddress, sdFlagUnicast);
    }
    else
    {
        // StopFind -> nothing to do
    }
}

// private
void SdMessageParser::handleEntryOffer(
    SdOptions const& options,
    service_id::type const serviceId,
    instance_id::type const instanceId,
    major_version::type const majorVersion,
    ttl::type const ttl,
    minor_version::type const minorVersion,
    ::ip::IPEndpoint const& sourceEndpoint)
{
    SdEndpoint const endpoint = searchIPEndpointOption(options, _localIp, _subnetId);

    if ((endpoint.isValid()) && (!::ip::isMulticastAddress(endpoint.getAddress())))
    {
        if (_additionalSDCheck && ((*_additionalSDCheck)(endpoint)))
        {
            return;
        }

        ServiceDescription const receivedService
            = {minorVersion,
               ttl,
               serviceId,
               instanceId,
               eventgroup_id::ALL,
               endpoint.getAddress(),
               endpoint.getPort(),
               endpoint.getProto(),
               majorVersion};

        _serviceRegistry.offerReceived(receivedService, sourceEndpoint.getAddress());
    }
}

// private
void SdMessageParser::handleEntrySubscribe(
    SdOptions const& options,
    service_id::type const serviceId,
    instance_id::type const instanceId,
    major_version::type const majorVersion,
    eventgroup_id::type const eventgroup,
    ttl::type const ttl,
    uint16_t const reserved,
    ::ip::IPEndpoint const& sourceEndpoint)
{
    SdEndpoint const endpoint = searchIPEndpointOption(options, _localIp, _subnetId);

    if (endpoint.isValid())
    {
        if (_additionalSDCheck && ((*_additionalSDCheck)(endpoint)))
        {
            _serviceAnnouncer.sendSubscribeNack(
                serviceId, instanceId, eventgroup, majorVersion, reserved, endpoint.getAddress());
            return;
        }

        _serviceAnnouncer.respondToSubscribe(
            serviceId,
            instanceId,
            majorVersion,
            reserved,
            eventgroup,
            ttl,
            sourceEndpoint.getAddress(),
            endpoint.getAddress(),
            endpoint.getPort(),
            endpoint.getProto());
    }
    else if (ttl != 0U) // don't Nack on StopSubscribe
    {
        _serviceAnnouncer.sendSubscribeNack(
            serviceId, instanceId, eventgroup, majorVersion, reserved, sourceEndpoint.getAddress());
    }
    else
    {
        ; // nothing else to do
    }
}

// private
void SdMessageParser::handleEntrySubscribeAck(
    SdOptions const& options,
    service_id::type const serviceId,
    instance_id::type const instanceId,
    major_version::type const majorVersion,
    eventgroup_id::type const eventgroup,
    ttl::type const ttl,
    ::ip::IPAddress const& sourceAddress)
{
    SdEndpoint const endpoint = searchIPMulticastOption(options);
    if (ttl != 0U)
    {
        Statistics::incCounter(Statistics::Counter::SD_SUBSCRIBE_ACK_RX);
        _serviceRegistry.subscribeAckReceived(
            serviceId, instanceId, eventgroup, majorVersion, endpoint, sourceAddress);
    }
    else
    {
        Statistics::incCounter(Statistics::Counter::SD_SUBSCRIBE_NACK_RX);
        _serviceRegistry.subscribeNackReceived(
            serviceId, instanceId, eventgroup, majorVersion, sourceAddress);
    }
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
