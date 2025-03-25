// Copyright 2024 Accenture.

#pragma once

#include "transport/TransportMessage.h"

#include <platform/estdint.h>

namespace transport
{
class TransportConfiguration
{
    TransportConfiguration();

public:
    static uint16_t const TESTER_RANGE_8BIT_START = 0x00F0U;
    static uint16_t const TESTER_RANGE_8BIT_END   = 0x00FDU;

    enum
    {
        INVALID_DIAG_ADDRESS = 0xFFU
    };

    static uint16_t const FUNCTIONAL_ALL_ISO14229             = 0x00DF;
    static uint16_t const MAX_FUNCTIONAL_MESSAGE_PAYLOAD_SIZE = 6U;
    static uint16_t const DIAG_PAYLOAD_SIZE                   = 4095U;

    static uint8_t const SERVICE_TERSTER_PRESENT    = 0x3E;
    static uint8_t const SUPPRESS_POSITIVE_RESPONSE = 0x80;

    static bool isFunctionalAddress(uint16_t address);

    static bool isFunctionallyAddressed(TransportMessage const& message);

    static bool is8BitTesterAddress(uint16_t address);

    /**
     * Checks is a given TransportMessages source id is a valid 8-bit tester id.
     */
    static bool isFromTester(TransportMessage const& message);

    static bool isTesterPresentWithSuppressPosResponse(TransportMessage const& message);
};

inline bool TransportConfiguration::isFunctionalAddress(uint16_t const address)
{
    return (FUNCTIONAL_ALL_ISO14229 == address);
}

inline bool TransportConfiguration::isFunctionallyAddressed(TransportMessage const& message)
{
    return isFunctionalAddress(message.getTargetId());
}

inline bool TransportConfiguration::is8BitTesterAddress(uint16_t const address)
{
    return (
        (address >= TransportConfiguration::TESTER_RANGE_8BIT_START)
        && (address <= TransportConfiguration::TESTER_RANGE_8BIT_END));
}

inline bool TransportConfiguration::isFromTester(TransportMessage const& message)
{
    return is8BitTesterAddress(message.getSourceId());
}

inline bool
TransportConfiguration::isTesterPresentWithSuppressPosResponse(TransportMessage const& message)
{
    return (
        (message.getServiceId() == TransportConfiguration::SERVICE_TERSTER_PRESENT)
        && (message.getPayload()[1] == TransportConfiguration::SUPPRESS_POSITIVE_RESPONSE));
}

} // namespace transport

