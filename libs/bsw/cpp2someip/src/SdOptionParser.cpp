/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SdOptionParser.h"

#include "someip/SdConstants.h"
#include "someip/SdOptions.h"
#include "someip/SomeIpConstants.h"
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
enum class OptionCategory : uint8_t
{
    IpEndpointOption,
    IpMulticastOption
};

bool isOptionCategory(OptionCategory const category, uint8_t const type)
{
    if (OptionCategory::IpEndpointOption == category)
    {
        return (type == static_cast<uint8_t>(ServiceDiscoveryOptionType::OPTION_TYPE_IP4_ENDPOINT))
               || (type
                   == static_cast<uint8_t>(ServiceDiscoveryOptionType::OPTION_TYPE_IP6_ENDPOINT));
    }
    if (OptionCategory::IpMulticastOption == category)
    {
        return (type == static_cast<uint8_t>(ServiceDiscoveryOptionType::OPTION_TYPE_IP4_MULTICAST))
               || (type
                   == static_cast<uint8_t>(ServiceDiscoveryOptionType::OPTION_TYPE_IP6_MULTICAST));
    }
    return false;
}

bool isIp4OptionType(uint8_t const type)
{
    return (type == static_cast<uint8_t>(ServiceDiscoveryOptionType::OPTION_TYPE_IP4_ENDPOINT))
           || (type == static_cast<uint8_t>(ServiceDiscoveryOptionType::OPTION_TYPE_IP4_MULTICAST));
}

bool isIp6OptionType(uint8_t const type)
{
    return (type == static_cast<uint8_t>(ServiceDiscoveryOptionType::OPTION_TYPE_IP6_ENDPOINT))
           || (type == static_cast<uint8_t>(ServiceDiscoveryOptionType::OPTION_TYPE_IP6_MULTICAST));
}

SdEndpoint
parseIpOptionType(::etl::span<uint8_t const> const buffer, uint16_t offset, uint8_t const type)
{
    offset += static_cast<uint16_t>(static_cast<uint16_t>(SdConstants::SD_OPTION_ADDRESS_OFFSET));

    ::ip::IPAddress ipAddr{};
    if (isIp4OptionType(type))
    {
        ipAddr = ::ip::make_ip4(
            ::etl::span<uint8_t const>(buffer).subspan(offset).first(::ip::IPAddress::IP4LENGTH));
        offset += ::ip::IPAddress::IP4LENGTH;
    }
#ifdef PLATFORM_SUPPORT_IPV6
    else if (isIp6OptionType(type))
    {
        ipAddr = ::ip::make_ip6(
            ::etl::span<uint8_t const>(buffer).subspan(offset).first(::ip::IPAddress::IP6LENGTH));
        offset += ::ip::IPAddress::IP6LENGTH;
    }
#endif
    else
    {
        return SdOptionParser::INVALID_ENDPOINT;
    }

    offset++; // reserved

    uint8_t const proto = buffer[offset];
    offset++;
    auto const port = ::someip::endian::read_be<uint16_t>(buffer.data() + offset);

    return SdEndpoint(ipAddr, port, proto);
}

bool goToOption(
    uint32_t const targetOptionsIndex, /* in */
    uint8_t const*& option,            /* inout */
    uint32_t const optionsLength,      /* in */
    uint8_t& optionCounter,            /* inout */
    uint32_t& offset)                  /* inout */
{
    while (optionCounter < targetOptionsIndex)
    {
        if ((optionsLength - offset)
            < static_cast<uint32_t>(SdConstants::SD_OPTION_LENGTH_FIELD_LENGTH))
        {
            return false;
        }
        auto const optionLength = ::someip::endian::read_be<uint16_t>(option);
        if ((optionsLength - offset)
            < (static_cast<uint32_t>(optionLength)
               + static_cast<uint32_t>(SdConstants::SD_OPTION_LENGTH_FIELD_LENGTH)
               + 1U)) // +1 because length does not cover type field
        {
            return false;
        }
        offset += static_cast<uint32_t>(optionLength) + 3U;
        option += static_cast<uint8_t>(optionLength + 3U);
        ++optionCounter;
    }
    return true;
}

bool searchOptions(
    OptionCategory const& optionCategory, /* in */
    uint8_t const*& option,               /* inout */
    uint32_t const optionsLength,         /* in */
    uint8_t const numAvailableOptions,    /* in */
    uint8_t& optionCounter,               /* inout */
    uint32_t& offset,                     /* inout */
    uint8_t const*& foundOption,          /* out */
    uint8_t& foundOptionType,             /* out */
    uint8_t& numUdpOptions,               /* out */
    uint8_t& numTcpOptions)               /* out */
{
    for (uint8_t i = 0U; i < numAvailableOptions; ++i)
    {
        if ((optionsLength - offset)
            < static_cast<uint32_t>(SdConstants::SD_OPTION_LENGTH_FIELD_LENGTH))
        {
            return false;
        }

        auto const optionLength = ::someip::endian::read_be<uint16_t>(option);
        if ((optionsLength - offset)
            < (static_cast<uint32_t>(optionLength)
               + static_cast<uint32_t>(SdConstants::SD_OPTION_LENGTH_FIELD_LENGTH)
               + 1U)) // +1 because length does not cover type field
        {
            return false;
        }

        uint8_t const optionType
            = option[static_cast<uint16_t>(SdConstants::SD_OPTION_TYPE_OFFSET)];
        if (!isOptionCategory(optionCategory, optionType))
        {
            offset += static_cast<uint32_t>(optionLength) + 3U;
            option += static_cast<uint8_t>(optionLength + 3U);
            ++optionCounter;
            continue; // not the one we are looking for
        }

        uint16_t payloadLength;
        uint16_t protoOffset;

        if (isIp4OptionType(optionType))
        {
            payloadLength = static_cast<uint16_t>(SdConstants::SD_IP4_OPTION_LENGTH);
            protoOffset   = static_cast<uint16_t>(
                static_cast<uint16_t>(SdConstants::SD_OPTION_ADDRESS_OFFSET)
                + ::ip::IPAddress::IP4LENGTH + 1U);
        }
        else if (isIp6OptionType(optionType))
        {
            payloadLength = static_cast<uint16_t>(SdConstants::SD_IP6_OPTION_LENGTH);
            protoOffset   = static_cast<uint16_t>(
                static_cast<uint16_t>(SdConstants::SD_OPTION_ADDRESS_OFFSET)
                + ::ip::IPAddress::IP6LENGTH + 1U);
        }
        else
        {
            WARN_LOG(SOMEIP, "SdOptionParser::searchOptions(type: %d) invalid type", optionType);
            return false;
        }

        if ((optionLength + 3U) != payloadLength)
        {
            WARN_LOG(
                SOMEIP,
                "SdOptionParser::searchOptions(type: %d, length: %d) invalid payload",
                optionType,
                payloadLength);
            return false;
        }

        if (proto::SD_L4_PROTO_UDP == option[protoOffset])
        {
            ++numUdpOptions;
            if ((numUdpOptions + numTcpOptions) == 1U)
            { // use first found option
                foundOption     = option;
                foundOptionType = optionType;
            }
        }
        else if (proto::SD_L4_PROTO_TCP == option[protoOffset])
        {
            ++numTcpOptions;
            if ((numUdpOptions + numTcpOptions) == 1U)
            { // use first found option
                foundOption     = option;
                foundOptionType = optionType;
            }
        }
        else
        {
            WARN_LOG(
                SOMEIP,
                "SdOptionParser::searchOptions(type: %d, proto: 0x%X) invalid proto",
                optionType,
                option[protoOffset]);
            return false;
        }

        offset += static_cast<uint32_t>(optionLength) + 3U;
        option += static_cast<uint8_t>(optionLength + 3U);
        ++optionCounter;
    }

    return true;
}

SdEndpoint parseIpOption(
    OptionCategory const& optionCategory,
    SdOptions const& options,
    uint8_t& numUdpOptions,
    uint8_t& numTcpOptions)
{
    ::etl::span<uint8_t const> const buffer = options.getOptionsData();

    uint8_t const* foundOption = nullptr;
    uint8_t foundOptionType    = 0U;

    {
        uint8_t const* option = buffer.data();
        uint8_t optionCounter = 0U;
        uint32_t offset       = 0U;

        // go to options1
        if (!goToOption(
                options.getOptions1Index(),
                option,
                static_cast<uint32_t>(buffer.size()),
                optionCounter,
                offset))
        {
            return SdOptionParser::INVALID_ENDPOINT;
        }

        // look at options1
        if (!searchOptions(
                optionCategory,
                option,
                static_cast<uint32_t>(buffer.size()),
                options.getOptions1Num(),
                optionCounter,
                offset,
                foundOption,
                foundOptionType,
                numUdpOptions,
                numTcpOptions))
        {
            return SdOptionParser::INVALID_ENDPOINT;
        }
    }

    {
        uint8_t const* option = buffer.data();
        uint8_t optionCounter = 0U;
        uint32_t offset       = 0U;

        // go to options2
        if (!goToOption(
                options.getOptions2Index(),
                option,
                static_cast<uint32_t>(buffer.size()),
                optionCounter,
                offset))
        {
            return SdOptionParser::INVALID_ENDPOINT;
        }

        // look at options2
        if (!searchOptions(
                optionCategory,
                option,
                static_cast<uint32_t>(buffer.size()),
                options.getOptions2Num(),
                optionCounter,
                offset,
                foundOption,
                foundOptionType,
                numUdpOptions,
                numTcpOptions))
        {
            return SdOptionParser::INVALID_ENDPOINT;
        }
    }

    if (foundOption == nullptr)
    {
        return SdOptionParser::INVALID_ENDPOINT;
    }

    return parseIpOptionType(
        buffer, static_cast<uint16_t>(foundOption - buffer.data()), foundOptionType);
}
} // namespace

// NOLINTNEXTLINE(cert-err58-cpp): Fine, since default value shall be used.
SdEndpoint const SdOptionParser::INVALID_ENDPOINT;

SdEndpoint SdOptionParser::parseIpEndpointOption(
    SdOptions const& options, uint8_t& numUdpOptions, uint8_t& numTcpOptions)
{
    return parseIpOption(OptionCategory::IpEndpointOption, options, numUdpOptions, numTcpOptions);
}

SdEndpoint SdOptionParser::parseIpMulticastOption(
    SdOptions const& options, uint8_t& numUdpOptions, uint8_t& numTcpOptions)
{
    return parseIpOption(OptionCategory::IpMulticastOption, options, numUdpOptions, numTcpOptions);
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
