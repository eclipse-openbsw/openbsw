/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <transport/LogicalAddress.h>
#include <transport/TransportMessage.h>

#include <etl/array.h>
#include <etl/span.h>

#include <array>

#include <platform/estdint.h>

namespace transport
{
class TransportConfiguration
{
public:
    TransportConfiguration() = delete;

    static constexpr size_t const NUMBER_OF_TESTER_ADDRESSES = 12U;
    static constexpr size_t const NUMBER_OF_ADDRESS_LISTS    = 1U;

    using LogicalAddressConverterGateway = LogicalAddressConverter<NUMBER_OF_ADDRESS_LISTS>;
    using TesterAddresses                = std::array<LogicalAddress, NUMBER_OF_TESTER_ADDRESSES>;

    static constexpr TesterAddresses const TESTER_ADDRESS_RANGE = {{
        {0x0EF0U, 0x00F0U},
        {0x0EF1U, 0x00F1U},
        {0x0EF2U, 0x00F2U},
        {0x0EF3U, 0x00F3U},
        {0x0EF4U, 0x00F4U},
        {0x0EF5U, 0x00F5U},
        {0x0EF6U, 0x00F6U},
        {0x0EF7U, 0x00F7U},
        {0x0EF8U, 0x00F8U},
        {0x0EF9U, 0x00F9U},
        {0x0EFAU, 0x00FAU},
        {0x0EFBU, 0x00FBU},
    }};

    /**
     * Functional addressing
     */
    static uint16_t const FUNCTIONAL_ALL_ISO14229 = 0x00DF;

    enum
    {
        INVALID_DIAG_ADDRESS = 0xFFU
    };

    /**
     * Maximum payload size for functionally addressed messages
     */
    static uint16_t const MAX_FUNCTIONAL_MESSAGE_PAYLOAD_SIZE = 6U;

    /**
     * Buffer size for diagnostic payload
     */
    static uint16_t const DIAG_PAYLOAD_SIZE = 4095U;

    /**
     * Number of full size transport messages
     */
    static uint8_t const NUMBER_OF_FULL_SIZE_TRANSPORT_MESSAGES = 5U;

    /**
     * Maximum number of transport messages
     */
    static uint8_t const MAXIMUM_NUMBER_OF_TRANSPORT_MESSAGES
        = NUMBER_OF_FULL_SIZE_TRANSPORT_MESSAGES * 8U;
    static bool isFunctionalAddress(uint16_t address);

    static bool isFunctionallyAddressed(TransportMessage const& message);

    static bool is2ByteTesterAddress(uint16_t address);

    static bool is1ByteTesterAddress(uint16_t address);

    static bool isTesterAddress(uint16_t address);

    static uint16_t convert2ByteAddressTo1Byte(uint16_t address);

    static uint16_t convert1ByteAddressTo2Byte(uint16_t address);

    static bool isFromTester(TransportMessage const& message);
};

/**
 * This function checks if the provided 16-bit address matches
 * the constant FUNCTIONAL_ALL_ISO14229.
 * \return true if they are equal and false otherwise.
 */
inline bool TransportConfiguration::isFunctionalAddress(uint16_t const address)
{
    return (FUNCTIONAL_ALL_ISO14229 == address);
}

/**
 * This function checks if the target ID of the provided TransportMessage
 * object matches the constant FUNCTIONAL_ALL_ISO14229.
 * \return true if they are equal and false otherwise.
 */
inline bool TransportConfiguration::isFunctionallyAddressed(TransportMessage const& message)
{
    return isFunctionalAddress(message.getTargetId());
}

inline bool TransportConfiguration::is2ByteTesterAddress(uint16_t const address)
{
    return addressfinder::is2ByteAddressIn(address, TESTER_ADDRESS_RANGE);
}

inline bool TransportConfiguration::is1ByteTesterAddress(uint16_t const address)
{
    return addressfinder::is1ByteAddressIn(address, TESTER_ADDRESS_RANGE);
}

/**
 * This function checks if the provided 2-byte address is a supported tester address.
 *
 * \return true if it is part of the bounded external tester set and false otherwise.
 */
inline bool TransportConfiguration::isTesterAddress(uint16_t const address)
{
    return is2ByteTesterAddress(address);
}

inline uint16_t TransportConfiguration::convert2ByteAddressTo1Byte(uint16_t const address)
{
    if ((address & 0xFF00U) != 0x0E00U)
    {
        return address;
    }

    return LogicalAddressConverterGateway::convert2ByteAddressTo1Byte(address);
}

inline uint16_t TransportConfiguration::convert1ByteAddressTo2Byte(uint16_t const address)
{
    if ((address & 0xFFF0U) != 0x00F0U)
    {
        return address;
    }

    return LogicalAddressConverterGateway::convert1ByteAddressTo2Byte(address);
}

/**
 * This function determines if the source ID of the provided TransportMessage object
 * corresponds to a 1-byte tester address.
 *
 * \return true if it does and false otherwise.
 */
inline bool TransportConfiguration::isFromTester(TransportMessage const& message)
{
    return is1ByteTesterAddress(message.getSourceId());
}

// Explicit template specialization declaration (definition in TransportConfiguration.cpp)
template<>
etl::array<::etl::span<LogicalAddress const>, TransportConfiguration::NUMBER_OF_ADDRESS_LISTS> const
    TransportConfiguration::LogicalAddressConverterGateway::TESTER_ADDRESS_LISTS;

} // namespace transport
