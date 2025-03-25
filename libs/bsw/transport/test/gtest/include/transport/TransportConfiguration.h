// Copyright 2024 Accenture.

/**
 * \file    TransportConfiguration.h
 */
#pragma once

#include "transport/LogicalAddress.h"
#include "transport/TransportMessage.h"

#include <platform/estdint.h>

namespace transport
{
class TransportConfiguration
{
public:
    static uint8_t const TESTER_RANGE_START = 0xF0U;
    static uint8_t const TESTER_RANGE_END   = 0xFDU;

    static constexpr size_t const NUMBER_OF_INTERNAL_TESTERS = 3;
    static constexpr size_t const NUMBER_OF_EXTERNAL_TESTERS = 2;

    static constexpr size_t const NUMBER_OF_ADDRESS_LISTS = 2;
    using LogicalAddressConverterUT = LogicalAddressConverter<NUMBER_OF_ADDRESS_LISTS>;

    using InternalTesters = std::array<LogicalAddress, NUMBER_OF_INTERNAL_TESTERS>;
    using ExternalTesters = std::array<LogicalAddress, NUMBER_OF_EXTERNAL_TESTERS>;
    static constexpr InternalTesters const INTERNAL_ADDRESS_RANGE
        = {{{0xABCDU, 0x00CDU}, {0x1234U, 0x0012U}, {0xF1F2U, 0x00FFU}}};
    static constexpr ExternalTesters const EXTERNAL_ADDRESS_RANGE
        = {{{0xA11DU, 0x0010U}, {0xDF01U, 0x0001U}}};

    static uint8_t const MAXIMUM_NUMBER_OF_TRANSPORT_MESSAGES = 10U;

    static uint8_t const SERVICE_TESTER_PRESENT     = 0x3E;
    static uint8_t const SUPPRESS_POSITIVE_RESPONSE = 0x80;

    static bool isFunctionalAddress(uint8_t address);

    static bool isFunctionallyAddressed(TransportMessage const& message);

    static bool isFromTester(TransportMessage const& message);

    static bool isTesterPresentWithSuppressPosResponse(TransportMessage const& message);
};

inline bool TransportConfiguration::isFunctionalAddress(uint8_t address)
{
    return (0xDF == address);
}

inline bool TransportConfiguration::isFunctionallyAddressed(TransportMessage const& message)
{
    return isFunctionalAddress(message.getTargetId());
}

inline bool TransportConfiguration::isFromTester(TransportMessage const& message)
{
    return (
        (message.getSourceId() >= TransportConfiguration::TESTER_RANGE_START)
        && (message.getSourceId() <= TransportConfiguration::TESTER_RANGE_END));
}

inline bool
TransportConfiguration::isTesterPresentWithSuppressPosResponse(TransportMessage const& message)
{
    return (
        (message.getServiceId() == TransportConfiguration::SERVICE_TESTER_PRESENT)
        && (message.getPayload()[1] == TransportConfiguration::SUPPRESS_POSITIVE_RESPONSE));
}

} // namespace transport
