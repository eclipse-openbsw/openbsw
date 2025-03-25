// Copyright 2024 Accenture.

#pragma once

#include <transport/TransportMessage.h>

#include <platform/estdint.h>

namespace transport
{
class TransportConfiguration
{
public:
    TransportConfiguration() = delete;

    /**
     * Tester address range for 8 bit CAN addressing
     */
    static uint16_t const TESTER_RANGE_8BIT_START = 0x00F0U;
    static uint16_t const TESTER_RANGE_8BIT_END   = 0x00FDU;

    /**
     * Tester address range for DOIP addressing
     */
    static uint16_t const TESTER_RANGE_DOIP_START = 0x0EF0U;
    static uint16_t const TESTER_RANGE_DOIP_END   = 0x0EFDU;

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

    static bool isTesterAddress(uint16_t address);

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

/**
 * This function checks if the provided 16-bit address falls within the predefined
 * ranges for tester addresses.
 * \return true if it falls within either the 8-bit or the DOIP range, and false otherwise.
 */
inline bool TransportConfiguration::isTesterAddress(uint16_t const address)
{
    return (
        ((address >= TransportConfiguration::TESTER_RANGE_8BIT_START)
         && (address <= TransportConfiguration::TESTER_RANGE_8BIT_END))
        || ((address >= TransportConfiguration::TESTER_RANGE_DOIP_START)
            && (address <= TransportConfiguration::TESTER_RANGE_DOIP_END)));
}

/**
 * This function determines if the source ID of the provided TransportMessage object
 * corresponds to a tester address based on predefined ranges.
 * \return true if it does and false otherwise.
 */
inline bool TransportConfiguration::isFromTester(TransportMessage const& message)
{
    return isTesterAddress(message.getSourceId());
}

} // namespace transport
