/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SdMessageBuilder.h"

#include "someip/SdConstants.h"
#include "someip/SdMessageConstants.h"
#include "someip/SomeIpConstants.h"
#include "someip/logger.h"

#include <ip/IPAddress.h>
#include <ip/to_str.h>

#include "someip/endian_helpers.h"
#include <etl/algorithm.h>
#include <etl/error_handler.h>
#include <etl/span.h>

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::util::logger::SOMEIP;

namespace
{
enum class MessageOffset : uint32_t
{
    // in order of appearance in memory
    MESSAGE_OFFSET_MESSAGE_ID        = 0U,
    MESSAGE_OFFSET_LENGTH            = 4U,
    MESSAGE_OFFSET_REQUEST_ID        = 8U,
    MESSAGE_OFFSET_PROTOCOL_VERSION  = 12U,
    MESSAGE_OFFSET_INTERFACE_VERSION = 13U,
    MESSAGE_OFFSET_MESSAGE_TYPE      = 14U,
    MESSAGE_OFFSET_RETURN_CODE       = 15U,
    MESSAGE_OFFSET_FLAGS             = 16U,
    MESSAGE_OFFSET_RESERVED          = 17U,
    MESSAGE_OFFSET_LENGTH_OF_ENTRIES = 20U,
    MESSAGE_OFFSET_ENTRIES           = 24U
};

enum class MessageEndpointOptionType : uint8_t
{
    MESSAGE_ENDPOINT_OPTION_TYPE_IPV4           = 0x04,
    MESSAGE_ENDPOINT_OPTION_TYPE_IPV6           = 0x06,
    MESSAGE_ENDPOINT_OPTION_TYPE_MULTICAST_IPV4 = 0x14,
    MESSAGE_ENDPOINT_OPTION_TYPE_MULTICAST_IPV6 = 0x16
};

enum : uint8_t
{
    INVALID_OPTION_INDEX = 0xFFU
};

enum class TtlConstants : uint32_t
{
    SET_ZERO_TTL = 0U,
    PRESERVE_TTL = 1U
};

using TtlHandling = TtlConstants;

uint32_t getEntriesLength(internal::Message const& message)
{
    return ::someip::endian::read_be<uint32_t>(
        &message.data[static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_LENGTH_OF_ENTRIES)]);
}

void setEntriesLength(internal::Message& message, uint32_t const length)
{
    ::someip::endian::write_be<uint32_t>(
        &message.data[static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_LENGTH_OF_ENTRIES)],
        length);
}

uint32_t getFreeBytesForEntries(internal::Message const& message)
{
    auto const entriesEnd
        = getEntriesLength(message) + static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_ENTRIES);
    return static_cast<uint32_t>(
        static_cast<uint8_t>(message.divider > entriesEnd) * (message.divider - entriesEnd));
}

uint32_t getOptionsLength(internal::Message const& message)
{
    return ::someip::endian::read_be<uint32_t>(&message.data[message.divider]);
}

void setOptionsLength(internal::Message& message, uint32_t const length)
{
    ::someip::endian::write_be<uint32_t>(&message.data[message.divider], length);
}

uint32_t getFreeBytesForOptions(internal::Message const& message)
{
    auto const optionsEnd = getOptionsLength(message) + message.divider
                            + static_cast<uint32_t>(static_cast<uint32_t>(
                                MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_OPTIONS_ARRAY));
    return static_cast<uint32_t>(
        static_cast<uint8_t>(message.data.size() > optionsEnd)
        * (message.data.size() - optionsEnd));
}

void writeOption(
    ::etl::span<uint8_t>& option,
    MessageEndpointOptionType const optionType,
    ::ip::IPAddress const& ipAddress,
    port::type const port,
    proto::type const proto)
{
    ::someip::endian::write_be<uint16_t>(&option[0], static_cast<uint16_t>(option.size() - 3U));
    option[2] = static_cast<uint8_t>(optionType);
    option[3] = 0U; // reserved
    option.advance(4);
    switch (optionType)
    {
        case MessageEndpointOptionType::MESSAGE_ENDPOINT_OPTION_TYPE_IPV4:
        case MessageEndpointOptionType::MESSAGE_ENDPOINT_OPTION_TYPE_MULTICAST_IPV4:
        {
            auto ipBytes = ::ip::ip4_bytes(ipAddress);
            etl::copy(ipBytes.begin(), ipBytes.end(), option.begin());
            option.advance(ipBytes.size());
            break;
        }
        case MessageEndpointOptionType::MESSAGE_ENDPOINT_OPTION_TYPE_IPV6:
        case MessageEndpointOptionType::MESSAGE_ENDPOINT_OPTION_TYPE_MULTICAST_IPV6:
        {
#ifdef PLATFORM_SUPPORT_IPV6
            auto ipBytes = ::ip::ip6_bytes(ipAddress);
            etl::copy(ipBytes.begin(), ipBytes.end(), option.begin());
            option.advance(ipBytes.size());
#else
            option.advance(::ip::IPAddress::IP6LENGTH);
#endif
            break;
        }
    }

    option[0] = 0U; // reserved
    option[1] = proto;
    ::someip::endian::write_be<uint16_t>(&option[2], port);
}

bool findEndpointOption(
    internal::Message const& message,
    ::ip::IPAddress const& ipAddress,
    port::type const port,
    proto::type const proto,
    uint8_t& optionIdx)
{
    if (::ip::isUnspecified(ipAddress))
    {
        optionIdx = INVALID_OPTION_INDEX;
        return true;
    }

    bool found               = false;
    size_t idx               = 0U;
    auto const optionsLength = getOptionsLength(message);
    ::ip::IPAddress writtenIpAddress{};
    auto const options = ::etl::span<uint8_t>(
        &message.data
             [message.divider
              + static_cast<uint32_t>(static_cast<uint32_t>(
                  MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_OPTIONS_ARRAY))],
        optionsLength);
    while (idx < optionsLength)
    {
        // skip option length
        idx += static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_LENGTH);
        MessageEndpointOptionType const optionType = static_cast<MessageEndpointOptionType>(
            ::someip::endian::read_be<uint8_t>(&options[idx]));
        idx += static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_TYPE)
               + static_cast<uint32_t>(
                   MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_RESERVED1);
        switch (optionType)
        {
            case MessageEndpointOptionType::MESSAGE_ENDPOINT_OPTION_TYPE_IPV4:
            case MessageEndpointOptionType::MESSAGE_ENDPOINT_OPTION_TYPE_MULTICAST_IPV4:
            {
                writtenIpAddress
                    = ::ip::make_ip4(::someip::endian::read_be<uint32_t>(&options[idx]));
                idx += static_cast<uint32_t>(
                           MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_IPV4ADDRESS)
                       + static_cast<uint32_t>(
                           MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_RESERVED2);
                break;
            }

            case MessageEndpointOptionType::MESSAGE_ENDPOINT_OPTION_TYPE_IPV6:
            case MessageEndpointOptionType::MESSAGE_ENDPOINT_OPTION_TYPE_MULTICAST_IPV6:
            {
#ifdef PLATFORM_SUPPORT_IPV6
                auto const ipv6slice = ::etl::span<uint8_t>(
                    &options[idx],
                    static_cast<uint32_t>(
                        MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_IPV6ADDRESS));
                writtenIpAddress = ::ip::make_ip6(ipv6slice);
#endif
                idx += static_cast<uint32_t>(
                           MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_IPV6ADDRESS)
                       + static_cast<uint32_t>(
                           MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_RESERVED2);
                break;
            }
        }
        auto const writtenProto = options[idx];
        idx += static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_PROTO);
        auto const writtenPort = ::someip::endian::read_be<uint16_t>(&options[idx]);
        idx += static_cast<uint32_t>(
            MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_PORT_NUMBER);
        found = (writtenIpAddress == ipAddress) && (proto == writtenProto) && (port == writtenPort);
        if (found)
        {
            return true;
        }
        ++optionIdx;
    }

    return false;
}

SdMessageReturnCode checkBuffer(::etl::span<uint8_t> const& buffer)
{
    enum : uint32_t
    {
        MESSAGE_CONSTANT_MINIMAL_MESSAGE_SIZE
        = static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_MESSAGE_ID)
          + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH)
          + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_REQUEST_ID)
          + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_PROTOCOL_VERSION)
          + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_INTERFACE_VERSION)
          + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_MESSAGE_TYPE)
          + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_RETURN_CODE)
          + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_FLAGS)
          + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_RESERVED)
          + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_ENTRIES_ARRAY)
          + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_OPTIONS_ARRAY)
    };

    // avoiding branching
    return static_cast<SdMessageReturnCode>(
        (static_cast<uint8_t>(MESSAGE_CONSTANT_MINIMAL_MESSAGE_SIZE > buffer.size())
         * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_BUFFER_TOO_SMALL))
        + (static_cast<uint8_t>(MESSAGE_CONSTANT_MINIMAL_MESSAGE_SIZE <= buffer.size())
           * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_OK)));
}

void resetMessage(
    internal::Message& message, ::etl::span<uint8_t> const& buffer = ::etl::span<uint8_t>())
{
    message.data = buffer;

    /*
     * Note: this part can be optimized in future in order to put the divider into the right place
     * to avoid as many memory moves as possible.
     */
    auto const maxOptionSize =
#ifdef PLATFORM_SUPPORT_IPV6
        static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_IPV6_OPTION_LENGTH);
#else
        static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_IPV4_OPTION_LENGTH);
#endif // OPENBSW_NO_IPV6

    // message.data.size() >= MESSAGE_CONSTANT_MINIMAL_MESSAGE_SIZE
    auto const freeBytes
        = message.data.size() - static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_ENTRIES)
          - static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_OPTIONS_ARRAY);

    // The same number of entries and options
    auto const n
        = freeBytes
          / (static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY) + maxOptionSize);

    message.divider = static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_ENTRIES)
                      + n * static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY);
}

uint16_t measureOption(
    ::ip::IPAddress const& ip, bool const multicast, MessageEndpointOptionType& optionType)
{
    optionType = static_cast<MessageEndpointOptionType>(
        // if it's ipv4
        (static_cast<uint8_t>(::ip::isIp4Address(ip))
         // set type to ipv4 multicast or endpoint
         * ((static_cast<uint8_t>(multicast)
             * static_cast<uint8_t>(
                 MessageEndpointOptionType::MESSAGE_ENDPOINT_OPTION_TYPE_MULTICAST_IPV4))
            + (static_cast<uint8_t>(!multicast)
               * static_cast<uint8_t>(
                   MessageEndpointOptionType::MESSAGE_ENDPOINT_OPTION_TYPE_IPV4))))
        // otherwise
        + (static_cast<uint8_t>(::ip::isIp6Address(ip))
           // set type to ipv6 multicast or endpoint
           * ((static_cast<uint8_t>(multicast)
               * static_cast<uint8_t>(
                   MessageEndpointOptionType::MESSAGE_ENDPOINT_OPTION_TYPE_MULTICAST_IPV6))
              + (static_cast<uint8_t>(!multicast)
                 * static_cast<uint8_t>(
                     MessageEndpointOptionType::MESSAGE_ENDPOINT_OPTION_TYPE_IPV6)))));

    // avoid branching while calculating option size
    return static_cast<uint8_t>(::ip::isIp4Address(ip))
               // if it's ipv4 return option size for ipv4
               * static_cast<uint32_t>(
                   MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_IPV4_OPTION_LENGTH)
           // return ipv6 option size otherwise
           + static_cast<uint8_t>(::ip::isIp6Address(ip))
                 * static_cast<uint32_t>(
                     MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_IPV6_OPTION_LENGTH);
}

void addHeader(internal::Message& message)
{
    uint8_t* ptr = &message.data[0U];
    ::someip::endian::write_be<uint32_t>(ptr, SD_MESSAGE_ID); // message ID
    ptr += sizeof(uint32_t);
    ::someip::endian::write_be<uint32_t>(ptr, 0); // message length (filled in later)
    ptr += sizeof(uint32_t);
    ::someip::endian::write_be<uint32_t>(ptr, 0); // client ID, session ID
    ptr += sizeof(uint32_t);
    ::someip::endian::write_be<uint32_t>(ptr, 0x01010200U); // versions, message type, return code
}

void writeFlags(internal::Message& message, bool const reboot)
{
    message.data[static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_FLAGS)]
        = static_cast<uint8_t>(SdFlags::SD_FLAG_UNICAST)
          | static_cast<uint8_t>(reboot) * static_cast<uint8_t>(SdFlags::SD_FLAG_REBOOT);
    ::someip::endian::write_be_24(
        &message.data[static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_RESERVED)],
        0U); // reserved
}

void writeSdOptions(
    ::etl::span<uint8_t, static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)> const&
        sdEntry,
    uint8_t const entry_type,
    uint8_t const option_idx_1 = 0U,
    uint8_t const options_num  = 0U)
{
    sdEntry[0] = entry_type;
    sdEntry[1] = option_idx_1;
    sdEntry[2] = 0U;
    sdEntry[3] = options_num;
}

void writeServiceDescription(
    ::etl::span<uint8_t, static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)> const&
        sdEntry,
    service_id::type const serviceId,
    instance_id::type const instanceId,
    major_version::type const majorVersion,
    minor_version::type const minorVersion,
    ttl::type const ttl,
    ttl::type const preserveTtl)
{
    ::someip::endian::write_be<uint16_t>(&sdEntry[4], serviceId);
    ::someip::endian::write_be<uint16_t>(&sdEntry[6], instanceId);
    sdEntry[8] = majorVersion;
    ::someip::endian::write_be_24(&sdEntry[9], preserveTtl * ttl);
    ::someip::endian::write_be<uint32_t>(&sdEntry[12], minorVersion);
}

SdMessageReturnCode writeOffer(
    internal::Message& message,
    service_id::type const serviceId,
    instance_id::type const instanceId,
    major_version::type const majorVersion,
    minor_version::type const minorVersion,
    ttl::type const ttl,
    uint8_t const optionIdx,
    TtlHandling const ttlHandling = TtlConstants::PRESERVE_TTL)
{
    auto const freeBytesForEntries = getFreeBytesForEntries(message);
    auto const optionsOffset       = static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_ENTRIES)
                               + getEntriesLength(message)
                               + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY);
    if (freeBytesForEntries < static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
    {
        // move options section to the right to fit a new entry
        (void)::memmove(
            &message.data[optionsOffset],
            &message.data[message.divider],
            static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_OPTIONS_ARRAY)
                + getOptionsLength(message));
        message.divider = optionsOffset;
    }

    auto const sdEntry
        = ::etl::span<uint8_t, static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)>(
            &message.data
                 [static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_ENTRIES)
                  + getEntriesLength(message)],
            static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY));

    writeSdOptions(sdEntry, ENTRY_TYPE_OFFER, optionIdx, 0x10U);
    writeServiceDescription(
        sdEntry,
        serviceId,
        instanceId,
        majorVersion,
        minorVersion,
        ttl,
        static_cast<uint32_t>(ttlHandling));

    setEntriesLength(
        message,
        getEntriesLength(message)
            + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY));
    return SdMessageReturnCode::SD_MESSAGE_OK;
}

void writeEventGroupDescription(
    ::etl::span<uint8_t, static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)> const&
        sdEntry,
    service_id::type const serviceId,
    instance_id::type const instanceId,
    eventgroup_id::type const eventGroup,
    major_version::type const majorVersion,
    ttl::type const ttl,
    uint32_t const preserveTtl)
{
    ::someip::endian::write_be<uint16_t>(&sdEntry[4], serviceId);
    ::someip::endian::write_be<uint16_t>(&sdEntry[6], instanceId);
    sdEntry[8] = majorVersion;
    ::someip::endian::write_be_24(&sdEntry[9], preserveTtl * ttl);
    ::someip::endian::write_be<uint16_t>(&sdEntry[12], 0U); // reserved
    ::someip::endian::write_be<uint16_t>(&sdEntry[14], eventGroup);
}

SdMessageReturnCode writePublish(
    internal::Message& message,
    service_id::type const serviceId,
    instance_id::type const instanceId,
    eventgroup_id::type const eventGroup,
    major_version::type const majorVersion,
    ttl::type const ttl,
    uint8_t const optionIndex,
    TtlHandling const ttlHandling = TtlConstants::PRESERVE_TTL)
{
    auto const freeBytesForEntries = getFreeBytesForEntries(message);
    if (freeBytesForEntries < static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
    {
        // move options section to the right to fit a new entry
        (void)::memmove(
            &message.data
                 [message.divider
                  + (static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                     - freeBytesForEntries)],
            &message.data[message.divider],
            static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_OPTIONS_ARRAY)
                + getOptionsLength(message));
        message.divider += static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                           - freeBytesForEntries;
    }
    auto const sdEntry
        = ::etl::span<uint8_t, static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)>(
            &message.data
                 [static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_ENTRIES)
                  + getEntriesLength(message)],
            static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY));

    writeSdOptions(sdEntry, ENTRY_TYPE_PUBLISH, optionIndex, 0x10U);
    writeEventGroupDescription(
        sdEntry,
        serviceId,
        instanceId,
        eventGroup,
        majorVersion,
        ttl,
        static_cast<uint32_t>(ttlHandling));

    setEntriesLength(
        message,
        getEntriesLength(message)
            + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY));
    return SdMessageReturnCode::SD_MESSAGE_OK;
}

void writeSubscription(
    ::etl::span<uint8_t, static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)> const&
        sdEntry,
    service_id::type const serviceId,
    instance_id::type const instanceId,
    eventgroup_id::type const eventGroup,
    major_version::type const majorVersion,
    minor_version::type const minorVersion,
    ttl::type const ttl,
    uint32_t const preserveTtl)
{
    ::someip::endian::write_be<uint16_t>(&sdEntry[4], serviceId);
    ::someip::endian::write_be<uint16_t>(&sdEntry[6], instanceId);
    sdEntry[8] = majorVersion;
    ::someip::endian::write_be_24(&sdEntry[9], preserveTtl * ttl);
    ::someip::endian::write_be<uint16_t>(&sdEntry[12], static_cast<uint16_t>(minorVersion));
    ::someip::endian::write_be<uint16_t>(&sdEntry[14], eventGroup);
}

SdMessageReturnCode associateEndpointOption(
    internal::Message& message,
    MessageEndpointOptionType const optionType,
    uint16_t const optionLength,
    ::ip::IPAddress const& ipAddress,
    port::type const port,
    proto::type const proto)
{
    if (proto == SomeIpConstants::INVALID_PROTO)
    {
        char addressStr[::ip::MAX_ENDPOINT_STRING_LENGTH];
        char* const addressStrPtr = ::ip::to_str(ipAddress, addressStr).data();
        WARN_LOG(
            SOMEIP,
            "SdMessageBuilder::associateEndpointOption(ip: %s, port: %d, proto: 0x%X) "
            "invalid proto",
            addressStrPtr,
            port,
            proto);
    }

    auto const freeBytesForOptions = getFreeBytesForOptions(message);
    if (freeBytesForOptions < optionLength)
    {
        // move options section to the left to fit a new option
        (void)::memmove(
            &message.data[message.divider - (optionLength - freeBytesForOptions)],
            &message.data[message.divider],
            static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_OPTIONS_ARRAY)
                + getOptionsLength(message));
        message.divider -= static_cast<uint16_t>(optionLength - freeBytesForOptions);
    }

    auto const optionsLength = getOptionsLength(message);
    auto option              = ::etl::span<uint8_t>(
        &message.data
             [message.divider
              + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_OPTIONS_ARRAY)
              + optionsLength],
        optionLength);
    writeOption(option, optionType, ipAddress, port, proto);
    setOptionsLength(message, getOptionsLength(message) + optionLength);
    return SdMessageReturnCode::SD_MESSAGE_OK;
}

SdMessageReturnCode associateMulticastOption(
    internal::Message& message,
    MessageEndpointOptionType const optionType,
    uint16_t const optionLength,
    ::ip::IPAddress const& ipAddress,
    port::type const port,
    proto::type const proto)
{
    auto const freeBytesForOptions = getFreeBytesForOptions(message);
    if (freeBytesForOptions < optionLength)
    {
        // move options section to the left to fit a new option
        (void)::memmove(
            &message.data[message.divider - (optionLength - freeBytesForOptions)],
            &message.data[message.divider],
            static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_OPTIONS_ARRAY)
                + getOptionsLength(message));
        message.divider -= static_cast<uint32_t>(optionLength - freeBytesForOptions);
    }

    auto const optionsLength = getOptionsLength(message);
    auto option              = ::etl::span<uint8_t>(
        &message.data
             [message.divider
              + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_OPTIONS_ARRAY)
              + optionsLength],
        optionLength);
    writeOption(option, optionType, ipAddress, port, proto);
    setOptionsLength(message, getOptionsLength(message) + optionLength);
    return SdMessageReturnCode::SD_MESSAGE_OK;
}
} // namespace

SdMessageReturnCode SdMessageBuilder::startMessage(::etl::span<uint8_t> const& buffer)
{
    SdMessageReturnCode const rc = checkBuffer(buffer);
    if (rc != SdMessageReturnCode::SD_MESSAGE_OK)
    {
        return rc;
    }

    resetMessage(_message, buffer);
    setEntriesLength(_message, 0U);
    setOptionsLength(_message, 0U);
    addHeader(_message);
    return rc;
}

SdMessageReturnCode SdMessageBuilder::addFind(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    major_version::type const majorVersion,
    minor_version::type const minorVersion,
    ttl::type const ttl)
{
    DEBUG_LOG(
        SOMEIP,
        "SdMessageBuilder::addFind(service: %d, version: %d, instance: %d)",
        serviceId,
        majorVersion,
        instanceId);

    auto const freeBytesForEntries = getFreeBytesForEntries(_message);
    if (freeBytesForEntries < static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
    {
        auto const freeBytesForOptions = getFreeBytesForOptions(_message);
        if ((freeBytesForEntries + freeBytesForOptions)
            < static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
        {
            ERROR_LOG(SOMEIP, "SdMessageBuilder::addSubscribeAck() message is too big");
            return SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE;
        }

        // move options section to the right to fit entry
        (void)::memmove(
            &_message.data
                 [_message.divider
                  + (static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                     - freeBytesForEntries)],
            &_message.data[_message.divider],
            static_cast<uint32_t>(
                static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_OPTIONS_ARRAY))
                + getOptionsLength(_message));

        _message.divider += static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                            - freeBytesForEntries;
    }

    auto const sdEntry
        = ::etl::span<uint8_t, static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)>(
            &_message.data
                 [static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_ENTRIES)
                  + getEntriesLength(_message)],
            static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY));

    writeSdOptions(sdEntry, ENTRY_TYPE_FIND);
    writeServiceDescription(
        sdEntry,
        serviceId,
        instanceId,
        majorVersion,
        minorVersion,
        ttl,
        static_cast<uint32_t>(TtlConstants::PRESERVE_TTL));

    setEntriesLength(
        _message,
        getEntriesLength(_message)
            + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY));

    auto const freeBytes = getFreeBytesForEntries(_message) + getFreeBytesForOptions(_message);
    // to avoid branching
    return static_cast<SdMessageReturnCode>(
        static_cast<uint8_t>(
            freeBytes < static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
            * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_IS_FULL)
        + static_cast<uint8_t>(
              freeBytes >= static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
              * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_OK));
}

SdMessageReturnCode SdMessageBuilder::addOffer(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    major_version::type const majorVersion,
    minor_version::type const minorVersion,
    ttl::type const ttl,
    ::ip::IPAddress const& ipAddress,
    port::type const port,
    proto::type const proto)
{
    DEBUG_LOG(
        SOMEIP,
        "SdMessageBuilder::addOffer(service: %d, version: %d, instance: %d)",
        serviceId,
        majorVersion,
        instanceId);

    uint8_t optionIdx      = 0U;
    auto const optionFound = findEndpointOption(_message, ipAddress, port, proto, optionIdx);
    MessageEndpointOptionType optionType;
    auto const optionLength    = measureOption(ipAddress, false, optionType);
    size_t const spaceRequired = static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                                 + static_cast<size_t>(!optionFound) * optionLength;
    if (((getFreeBytesForEntries(_message) + getFreeBytesForOptions(_message))
         < static_cast<uint32_t>(spaceRequired))
        || (optionIdx == INVALID_OPTION_INDEX))
    {
        // avoid branching
        return static_cast<SdMessageReturnCode>(
            static_cast<uint8_t>(optionIdx == INVALID_OPTION_INDEX)
                * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_INVALID_ADDRESS)
            + static_cast<uint8_t>(optionIdx != INVALID_OPTION_INDEX)
                  * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE));
    }

    auto rc
        = writeOffer(_message, serviceId, instanceId, majorVersion, minorVersion, ttl, optionIdx);
    if ((rc == SdMessageReturnCode::SD_MESSAGE_OK) && (!optionFound))
    {
        rc = associateEndpointOption(_message, optionType, optionLength, ipAddress, port, proto);
    }

    return rc;
}

SdMessageReturnCode SdMessageBuilder::addDenounce(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    major_version::type const majorVersion,
    minor_version::type const minorVersion,
    ttl::type const ttl,
    ::ip::IPAddress const& ipAddress,
    port::type const port,
    proto::type const proto)
{
    DEBUG_LOG(
        SOMEIP,
        "SdMessageBuilder::addDenounce(service: %d, version: %d, instance: %d)",
        serviceId,
        majorVersion,
        instanceId);

    uint8_t optionIdx      = 0U;
    auto const optionFound = findEndpointOption(_message, ipAddress, port, proto, optionIdx);
    MessageEndpointOptionType optionType;
    auto const optionLength    = measureOption(ipAddress, false, optionType);
    size_t const spaceRequired = static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                                 + static_cast<size_t>(!optionFound) * optionLength;
    if (((getFreeBytesForEntries(_message) + getFreeBytesForOptions(_message))
         < static_cast<uint32_t>(spaceRequired))
        || (optionIdx == INVALID_OPTION_INDEX))
    {
        // avoid branching
        return static_cast<SdMessageReturnCode>(
            static_cast<uint8_t>(optionIdx == INVALID_OPTION_INDEX)
                * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_INVALID_ADDRESS)
            + static_cast<uint8_t>(optionIdx != INVALID_OPTION_INDEX)
                  * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE));
    }

    auto rc = writeOffer(
        _message,
        serviceId,
        instanceId,
        majorVersion,
        minorVersion,
        ttl,
        optionIdx,
        TtlConstants::SET_ZERO_TTL);
    if ((rc == SdMessageReturnCode::SD_MESSAGE_OK) && (!optionFound))
    {
        rc = associateEndpointOption(_message, optionType, optionLength, ipAddress, port, proto);
    }

    return rc;
}

SdMessageReturnCode SdMessageBuilder::addFindEventgroup(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    eventgroup_id::type const eventGroup,
    major_version::type const majorVersion,
    ttl::type const ttl)
{
    DEBUG_LOG(
        SOMEIP,
        "SdMessageBuilder::addFindEventgroup(service: %d, version: %d, instance: %d, eventgroup: "
        "%d)",
        serviceId,
        majorVersion,
        instanceId,
        eventGroup);

    auto const freeBytesForEntries = getFreeBytesForEntries(_message);
    if (freeBytesForEntries < static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
    {
        auto const freeBytesForOptions = getFreeBytesForOptions(_message);
        if ((freeBytesForEntries + freeBytesForOptions)
            < static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
        {
            ERROR_LOG(SOMEIP, "SdMessageBuilder::addFindEventgroup() message is too big");
            return SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE;
        }

        // move options section to the right to fit entry
        (void)::memmove(
            &_message.data
                 [_message.divider
                  + (static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                     - freeBytesForEntries)],
            &_message.data[_message.divider],
            static_cast<uint32_t>(
                static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_OPTIONS_ARRAY))
                + getOptionsLength(_message));

        _message.divider += static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                            - freeBytesForEntries;
    }

    auto const entry
        = ::etl::span<uint8_t, static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)>(
            &_message.data
                 [static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_ENTRIES)
                  + getEntriesLength(_message)],
            static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY));

    writeSdOptions(entry, ENTRY_TYPE_FIND_EVENTGROUP);
    writeEventGroupDescription(
        entry,
        serviceId,
        instanceId,
        eventGroup,
        majorVersion,
        ttl,
        static_cast<uint32_t>(TtlConstants::PRESERVE_TTL));

    setEntriesLength(
        _message,
        getEntriesLength(_message)
            + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY));

    auto const freeBytes = getFreeBytesForEntries(_message) + getFreeBytesForOptions(_message);
    return static_cast<SdMessageReturnCode>(
        static_cast<uint8_t>(
            freeBytes < static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
            * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_IS_FULL)
        + static_cast<uint8_t>(
              freeBytes >= static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
              * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_OK));
}

SdMessageReturnCode SdMessageBuilder::addPublish(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    eventgroup_id::type const eventGroup,
    major_version::type const majorVersion,
    ttl::type const ttl,
    ::ip::IPAddress const& ipAddress,
    port::type const port,
    proto::type const proto)
{
    DEBUG_LOG(
        SOMEIP,
        "SdMessageBuilder::addPublish(service: %d, version: %d, instance: %d, eventgroup: %d)",
        serviceId,
        majorVersion,
        instanceId,
        eventGroup);

    uint8_t optionIdx      = 0U;
    auto const optionFound = findEndpointOption(_message, ipAddress, port, proto, optionIdx);
    MessageEndpointOptionType optionType;
    auto const optionLength    = measureOption(ipAddress, false, optionType);
    size_t const spaceRequired = static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                                 + static_cast<size_t>(!optionFound) * optionLength;
    if (((getFreeBytesForEntries(_message) + getFreeBytesForOptions(_message))
         < static_cast<uint32_t>(spaceRequired))
        || (optionIdx == INVALID_OPTION_INDEX))
    {
        // avoid branching
        return static_cast<SdMessageReturnCode>(
            static_cast<uint8_t>(optionIdx == INVALID_OPTION_INDEX)
                * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_INVALID_ADDRESS)
            + static_cast<uint8_t>(optionIdx != INVALID_OPTION_INDEX)
                  * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE));
    }

    auto rc
        = writePublish(_message, serviceId, instanceId, eventGroup, majorVersion, ttl, optionIdx);

    if (!optionFound)
    {
        rc = associateEndpointOption(_message, optionType, optionLength, ipAddress, port, proto);
    }

    auto const freeBytes = getFreeBytesForEntries(_message) + getFreeBytesForOptions(_message);
    return (rc == SdMessageReturnCode::SD_MESSAGE_OK) ? (static_cast<SdMessageReturnCode>(
               static_cast<uint8_t>(
                   freeBytes < static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
                   * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_IS_FULL)
               + static_cast<uint8_t>(
                     freeBytes >= static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
                     * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_OK)))
                                                      : rc;
}

SdMessageReturnCode SdMessageBuilder::addUnpublish(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    eventgroup_id::type const eventGroup,
    major_version::type const majorVersion,
    ttl::type const ttl,
    ::ip::IPAddress const& ipAddress,
    port::type const port,
    proto::type const proto)
{
    DEBUG_LOG(
        SOMEIP,
        "SdMessageBuilder::addUnpublish(service: %d, version: %d, instance: %d, eventgroup: %d)",
        serviceId,
        majorVersion,
        instanceId,
        eventGroup);

    uint8_t optionIdx      = 0U;
    auto const optionFound = findEndpointOption(_message, ipAddress, port, proto, optionIdx);
    MessageEndpointOptionType optionType;
    auto const optionLength    = measureOption(ipAddress, false, optionType);
    size_t const spaceRequired = static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                                 + static_cast<size_t>(!optionFound) * optionLength;
    if (((getFreeBytesForEntries(_message) + getFreeBytesForOptions(_message))
         < static_cast<uint32_t>(spaceRequired))
        || (optionIdx == INVALID_OPTION_INDEX))
    {
        // avoid branching
        return static_cast<SdMessageReturnCode>(
            static_cast<uint8_t>(optionIdx == INVALID_OPTION_INDEX)
                * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_INVALID_ADDRESS)
            + static_cast<uint8_t>(optionIdx != INVALID_OPTION_INDEX)
                  * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE));
    }

    auto rc = writePublish(
        _message,
        serviceId,
        instanceId,
        eventGroup,
        majorVersion,
        ttl,
        optionIdx,
        TtlConstants::SET_ZERO_TTL);

    if (!optionFound)
    {
        rc = associateEndpointOption(_message, optionType, optionLength, ipAddress, port, proto);
    }

    return rc;
}

SdMessageReturnCode SdMessageBuilder::addSubscribe(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    eventgroup_id::type const eventGroup,
    major_version::type const majorVersion,
    ttl::type const ttl,
    ::ip::IPAddress const& ipAddress,
    port::type const port,
    proto::type const proto)
{
    DEBUG_LOG(
        SOMEIP,
        "SdMessageBuilder::addSubscribe(service: %d, version: %d, instance: %d, eventgroup: %d)",
        serviceId,
        majorVersion,
        instanceId,
        eventGroup);

    uint8_t optionIdx      = 0U;
    auto const optionFound = findEndpointOption(_message, ipAddress, port, proto, optionIdx);
    MessageEndpointOptionType optionType;
    auto const optionLength    = measureOption(ipAddress, false, optionType);
    size_t const spaceRequired = static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                                 + static_cast<size_t>(!optionFound) * optionLength;
    auto const freeBytesForEntries = getFreeBytesForEntries(_message);
    auto const freeBytesForOptions = getFreeBytesForOptions(_message);
    if (((freeBytesForEntries + freeBytesForOptions) < static_cast<uint32_t>(spaceRequired))
        || (optionIdx == INVALID_OPTION_INDEX))
    {
        // avoid branching
        return static_cast<SdMessageReturnCode>(
            static_cast<uint8_t>(optionIdx == INVALID_OPTION_INDEX)
                * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_INVALID_ADDRESS)
            + static_cast<uint8_t>(optionIdx != INVALID_OPTION_INDEX)
                  * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE));
    }

    if (freeBytesForEntries < static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
    {
        // move options section to the right to fit the entry
        (void)::memmove(
            &_message.data
                 [_message.divider
                  + (static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                     - freeBytesForEntries)],
            &_message.data[_message.divider],
            static_cast<uint32_t>(
                static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_OPTIONS_ARRAY))
                + getOptionsLength(_message));

        _message.divider += static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                            - freeBytesForEntries;
    }

    auto const entry
        = ::etl::span<uint8_t, static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)>(
            &_message.data
                 [static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_ENTRIES)
                  + getEntriesLength(_message)],
            static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY));

    writeSdOptions(entry, ENTRY_TYPE_SUBSCRIBE, optionIdx, 0x10U);
    writeEventGroupDescription(
        entry,
        serviceId,
        instanceId,
        eventGroup,
        majorVersion,
        ttl,
        static_cast<uint32_t>(TtlConstants::PRESERVE_TTL));

    setEntriesLength(
        _message,
        getEntriesLength(_message)
            + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY));

    auto rc = SdMessageReturnCode::SD_MESSAGE_OK;
    if (!optionFound)
    {
        rc = associateEndpointOption(_message, optionType, optionLength, ipAddress, port, proto);
    }

    auto const freeBytes = getFreeBytesForEntries(_message) + getFreeBytesForOptions(_message);
    return (rc == SdMessageReturnCode::SD_MESSAGE_OK) ? (static_cast<SdMessageReturnCode>(
               static_cast<uint8_t>(
                   freeBytes < static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
                   * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_IS_FULL)
               + static_cast<uint8_t>(
                     freeBytes >= static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
                     * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_OK)))
                                                      : rc;
}

SdMessageReturnCode SdMessageBuilder::addUnsubscribe(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    eventgroup_id::type const eventGroup,
    major_version::type const majorVersion,
    ttl::type const ttl,
    ::ip::IPAddress const& ipAddress,
    port::type const port,
    proto::type const proto)
{
    DEBUG_LOG(
        SOMEIP,
        "SdMessageBuilder::addUnsubscribe(service: %d, version: %d, instance: %d, eventgroup: %d)",
        serviceId,
        majorVersion,
        instanceId,
        eventGroup);

    uint8_t optionIdx      = 0U;
    auto const optionFound = findEndpointOption(_message, ipAddress, port, proto, optionIdx);
    MessageEndpointOptionType optionType;
    auto const optionLength    = measureOption(ipAddress, false, optionType);
    size_t const spaceRequired = static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                                 + static_cast<size_t>(!optionFound) * optionLength;
    if (((getFreeBytesForEntries(_message) + getFreeBytesForOptions(_message))
         < static_cast<uint32_t>(spaceRequired))
        || (optionIdx == INVALID_OPTION_INDEX))
    {
        // avoid branching
        return static_cast<SdMessageReturnCode>(
            static_cast<uint8_t>(optionIdx == INVALID_OPTION_INDEX)
                * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_INVALID_ADDRESS)
            + static_cast<uint8_t>(optionIdx != INVALID_OPTION_INDEX)
                  * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE));
    }

    auto const entry
        = ::etl::span<uint8_t, static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)>(
            &_message.data
                 [static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_ENTRIES)
                  + getEntriesLength(_message)],
            static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY));

    writeSdOptions(entry, ENTRY_TYPE_SUBSCRIBE, optionIdx, 0x10U);
    writeEventGroupDescription(
        entry,
        serviceId,
        instanceId,
        eventGroup,
        majorVersion,
        ttl,
        static_cast<uint32_t>(TtlConstants::SET_ZERO_TTL));

    setEntriesLength(
        _message,
        getEntriesLength(_message)
            + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY));

    auto rc = SdMessageReturnCode::SD_MESSAGE_OK;
    if (!optionFound)
    {
        rc = associateEndpointOption(_message, optionType, optionLength, ipAddress, port, proto);
    }

    auto const freeBytes = getFreeBytesForEntries(_message) + getFreeBytesForOptions(_message);
    // to avoid branching
    return (rc == SdMessageReturnCode::SD_MESSAGE_OK) ? (static_cast<SdMessageReturnCode>(
               static_cast<uint8_t>(
                   freeBytes < static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
                   * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_IS_FULL)
               + static_cast<uint8_t>(
                     freeBytes >= static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
                     * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_OK)))
                                                      : rc;
}

SdMessageReturnCode SdMessageBuilder::addSubscribeAck(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    eventgroup_id::type const eventGroup,
    major_version::type const majorVersion,
    minor_version::type const minorVersion,
    ttl::type const ttl)
{
    DEBUG_LOG(
        SOMEIP,
        "SdMessageBuilder::addSubscribeAck(service: %d, version: %d, instance: %d, eventgroup: %d)",
        serviceId,
        majorVersion,
        instanceId,
        eventGroup);

    auto const freeBytesForEntries = getFreeBytesForEntries(_message);
    if (freeBytesForEntries < static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
    {
        auto const freeBytesForOptions = getFreeBytesForOptions(_message);
        if ((freeBytesForEntries + freeBytesForOptions)
            < static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
        {
            ERROR_LOG(SOMEIP, "SdMessageBuilder::addSubscribeAck() message is too big");
            return SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE;
        }

        // move options section to the right to fit the entry
        (void)::memmove(
            &_message.data
                 [_message.divider
                  + (static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                     - freeBytesForEntries)],
            &_message.data[_message.divider],
            static_cast<uint32_t>(
                static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_OPTIONS_ARRAY))
                + getOptionsLength(_message));

        _message.divider += static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                            - freeBytesForEntries;
    }

    auto const entry
        = ::etl::span<uint8_t, static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)>(
            &_message.data
                 [static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_ENTRIES)
                  + getEntriesLength(_message)],
            static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY));

    writeSdOptions(entry, ENTRY_TYPE_SUBSCRIBE_ACK);
    writeSubscription(
        entry,
        serviceId,
        instanceId,
        eventGroup,
        majorVersion,
        minorVersion,
        ttl,
        static_cast<uint32_t>(TtlConstants::PRESERVE_TTL));

    setEntriesLength(
        _message,
        getEntriesLength(_message)
            + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY));

    auto const freeBytes = getFreeBytesForEntries(_message) + getFreeBytesForOptions(_message);
    // to avoid branching
    return static_cast<SdMessageReturnCode>(
        static_cast<uint8_t>(
            freeBytes < static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
            * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_IS_FULL)
        + static_cast<uint8_t>(
              freeBytes >= static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
              * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_OK));
}

SdMessageReturnCode SdMessageBuilder::addSubscribeNack(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    eventgroup_id::type const eventGroup,
    major_version::type const majorVersion,
    minor_version::type const minorVersion,
    ttl::type const ttl)
{
    DEBUG_LOG(
        SOMEIP,
        "SdMessageBuilder::addSubscribeNack(service: %d, version: %d, instance: %d, eventgroup: "
        "%d)",
        serviceId,
        majorVersion,
        instanceId,
        eventGroup);

    auto const freeBytesForEntries = getFreeBytesForEntries(_message);
    if (freeBytesForEntries < static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
    {
        auto const freeBytesForOptions = getFreeBytesForOptions(_message);
        if ((freeBytesForEntries + freeBytesForOptions)
            < static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
        {
            ERROR_LOG(SOMEIP, "SdMessageBuilder::addSubscribeNack() message is too big");
            return SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE;
        }

        // move options section to the right to fit entry
        (void)::memmove(
            &_message.data
                 [_message.divider
                  + (static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                     - freeBytesForEntries)],
            &_message.data[_message.divider],
            static_cast<uint32_t>(
                static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_OPTIONS_ARRAY))
                + getOptionsLength(_message));

        _message.divider += static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                            - freeBytesForEntries;
    }

    auto const entry
        = ::etl::span<uint8_t, static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)>(
            &_message.data
                 [static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_ENTRIES)
                  + getEntriesLength(_message)],
            static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY));

    writeSdOptions(entry, ENTRY_TYPE_SUBSCRIBE_ACK);
    writeSubscription(
        entry,
        serviceId,
        instanceId,
        eventGroup,
        majorVersion,
        minorVersion,
        ttl,
        static_cast<uint32_t>(TtlConstants::SET_ZERO_TTL));

    setEntriesLength(
        _message,
        getEntriesLength(_message)
            + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY));

    auto const freeBytes = getFreeBytesForEntries(_message) + getFreeBytesForOptions(_message);
    // to avoid branching
    return static_cast<SdMessageReturnCode>(
        static_cast<uint8_t>(
            freeBytes < static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
            * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_IS_FULL)
        + static_cast<uint8_t>(
              freeBytes >= static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
              * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_OK));
}

SdMessageReturnCode SdMessageBuilder::addSubscribeAckMulticast(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    eventgroup_id::type const eventGroup,
    major_version::type const majorVersion,
    minor_version::type const minorVersion,
    ttl::type const ttl,
    ::ip::IPAddress const& ipAddress,
    port::type const port,
    proto::type const proto)
{
    DEBUG_LOG(
        SOMEIP,
        "SdMessageBuilder::addSubscribeAckMulticast(service: %d, version: %d, instance: %d, "
        "eventgroup: %d)",
        serviceId,
        majorVersion,
        instanceId,
        eventGroup);

    uint8_t optionIdx      = 0U;
    auto const optionFound = findEndpointOption(_message, ipAddress, port, proto, optionIdx);
    MessageEndpointOptionType optionType;
    auto const optionLength    = measureOption(ipAddress, true, optionType);
    size_t const spaceRequired = static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                                 + static_cast<size_t>(!optionFound) * optionLength;
    auto const freeBytesForEntries = getFreeBytesForEntries(_message);
    auto const freeBytesForOptions = getFreeBytesForOptions(_message);
    if (((freeBytesForEntries + freeBytesForOptions) < static_cast<uint32_t>(spaceRequired))
        || (optionIdx == INVALID_OPTION_INDEX))
    {
        // avoid branching
        return static_cast<SdMessageReturnCode>(
            static_cast<uint8_t>(optionIdx == INVALID_OPTION_INDEX)
                * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_INVALID_ADDRESS)
            + static_cast<uint8_t>(optionIdx != INVALID_OPTION_INDEX)
                  * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE));
    }

    if (freeBytesForEntries < static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
    {
        // move options section to the right to fit the entry
        (void)::memmove(
            &_message.data
                 [_message.divider
                  + (static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                     - freeBytesForEntries)],
            &_message.data[_message.divider],
            static_cast<uint32_t>(
                static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_OPTIONS_ARRAY))
                + getOptionsLength(_message));

        _message.divider += static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                            - freeBytesForEntries;
    }

    auto const entry
        = ::etl::span<uint8_t, static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)>(
            &_message.data
                 [static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_ENTRIES)
                  + getEntriesLength(_message)],
            static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY));

    writeSdOptions(entry, ENTRY_TYPE_SUBSCRIBE_ACK, optionIdx, 0x10U);
    writeSubscription(
        entry,
        serviceId,
        instanceId,
        eventGroup,
        majorVersion,
        minorVersion,
        ttl,
        static_cast<uint32_t>(TtlConstants::PRESERVE_TTL));

    setEntriesLength(
        _message,
        getEntriesLength(_message)
            + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY));

    auto rc = SdMessageReturnCode::SD_MESSAGE_OK;
    if (!optionFound)
    {
        rc = associateMulticastOption(_message, optionType, optionLength, ipAddress, port, proto);
    }

    auto const freeBytes = getFreeBytesForEntries(_message) + getFreeBytesForOptions(_message);
    // to avoid branching
    return (rc == SdMessageReturnCode::SD_MESSAGE_OK) ? (static_cast<SdMessageReturnCode>(
               static_cast<uint8_t>(
                   freeBytes < static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
                   * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_IS_FULL)
               + static_cast<uint8_t>(
                     freeBytes >= static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY))
                     * static_cast<uint8_t>(SdMessageReturnCode::SD_MESSAGE_OK)))
                                                      : rc;
}

::etl::span<uint8_t const>
SdMessageBuilder::finishMessage(uint16_t const sessionId, bool const reboot)
{
    if (_message.data.size() == 0)
    {
        return {};
    }

    writeFlags(_message, reboot);

    auto const optionsLength = getOptionsLength(_message);
    auto const entriesLength = getEntriesLength(_message);
    if (static_cast<uint32_t>(_message.divider)
        != (static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_ENTRIES) + entriesLength))
    {
        (void)::memmove(
            &_message.data
                 [static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_ENTRIES) + entriesLength],
            &_message.data[_message.divider],
            static_cast<uint32_t>(
                static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_OPTIONS_ARRAY))
                + optionsLength);

        _message.divider
            = static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_ENTRIES) + entriesLength;
    }

    auto const messageLength
        = static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_ENTRIES) + getEntriesLength(_message)
          + static_cast<uint32_t>(
              static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_OPTIONS_ARRAY))
          + getOptionsLength(_message);
    ::someip::endian::write_be<uint32_t>(
        &_message.data[static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_LENGTH)],
        messageLength
            - (static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH)
               + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_MESSAGE_ID)));

    // update session id
    ::someip::endian::write_be<uint32_t>(
        &_message.data[static_cast<uint32_t>(MessageOffset::MESSAGE_OFFSET_REQUEST_ID)], sessionId);

    auto const fullMessage = _message.data.first(messageLength);
    _message.data          = decltype(_message.data){};

    return fullMessage;
}

void SdMessageBuilder::discardMessage() { _message.data = decltype(_message.data){}; }

bool SdMessageBuilder::isEmpty() const { return getEntriesLength(_message) == 0U; }

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
