/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "busid/BusId.h"

#include "transport/LogicalAddress.h"
#include "transport/TransportMessage.h"

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

    static constexpr size_t NUMBER_OF_ETHERNET_TESTERS = 3;
    static constexpr size_t NUMBER_OF_CAN_TESTERS      = 2;
    static constexpr size_t NUMBER_OF_ADDRESS_LISTS    = 2;

    using EthernetTesters           = std::array<LogicalAddress, NUMBER_OF_ETHERNET_TESTERS>;
    using CanTesters                = std::array<LogicalAddress, NUMBER_OF_CAN_TESTERS>;
    using LogicalAddressConverterUT = LogicalAddressConverter<NUMBER_OF_ADDRESS_LISTS>;

    static constexpr EthernetTesters TESTER_ADDRESS_RANGE_ETHERNET
        = {{{0x0ECDU, 0x00F0U}, {0x0E12U, 0x00F1U}, {0x0EFFU, 0x00F2U}}};
    static constexpr CanTesters TESTER_ADDRESS_RANGE_CAN
        = {{{0x0E10U, 0x00F3U}, {0x0E01U, 0x00F4U}}};

    /**
     * Functional addressing
     */
    static uint16_t const FUNCTIONAL_ALL_ISO14229 = 0x00DFU;

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

    /// True if the bus uses 1-byte diagnostic addresses (i.e. not a DoIP bus).
    static bool is1ByteDiagAddressBus(uint8_t busId);

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

inline bool TransportConfiguration::is1ByteDiagAddressBus(uint8_t const busId)
{
    // Only the Ethernet/DoIP buses use 2-byte diagnostic addresses.
    return (busId != ::busid::ETH_0) && (busId != ::busid::ETH_1);
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
    return addressfinder::is2ByteAddressIn(address, TESTER_ADDRESS_RANGE_ETHERNET)
           || addressfinder::is2ByteAddressIn(address, TESTER_ADDRESS_RANGE_CAN);
}

inline bool TransportConfiguration::is1ByteTesterAddress(uint16_t const address)
{
    return addressfinder::is1ByteAddressIn(address, TESTER_ADDRESS_RANGE_ETHERNET)
           || addressfinder::is1ByteAddressIn(address, TESTER_ADDRESS_RANGE_CAN);
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

    return LogicalAddressConverterUT::convert2ByteAddressTo1Byte(address);
}

inline uint16_t TransportConfiguration::convert1ByteAddressTo2Byte(uint16_t const address)
{
    if ((address & 0xFFF0U) != 0x00F0U)
    {
        return address;
    }

    return LogicalAddressConverterUT::convert1ByteAddressTo2Byte(address);
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
    TransportConfiguration::LogicalAddressConverterUT::TESTER_ADDRESS_LISTS;

} // namespace transport
