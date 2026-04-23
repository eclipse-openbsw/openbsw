// Copyright 2026 Accenture.

#pragma once

#include "transport/LogicalAddress.h"
#include "transport/TransportMessage.h"

#include <platform/estdint.h>

#include <array>

namespace transport
{
class TransportConfiguration
{
public:
    TransportConfiguration() = delete;

    static uint16_t const TESTER_RANGE_START = 0x00F0U;
    static uint16_t const TESTER_RANGE_END   = 0x00FDU;

    static uint16_t const FUNCTIONAL_ALL_ISO14229             = 0x00DFU;
    static uint16_t const MAX_FUNCTIONAL_MESSAGE_PAYLOAD_SIZE = 6U;
    static uint16_t const DIAG_PAYLOAD_SIZE                   = 4095U;

    enum
    {
        INVALID_DIAG_ADDRESS = 0xFFU
    };

    static uint8_t const NUMBER_OF_FULL_SIZE_TRANSPORT_MESSAGES = 5U;
    static uint8_t const MAXIMUM_NUMBER_OF_TRANSPORT_MESSAGES
        = NUMBER_OF_FULL_SIZE_TRANSPORT_MESSAGES * 8U;

    static constexpr size_t NUMBER_OF_ETHERNET_TESTERS = 3;
    static constexpr size_t NUMBER_OF_CAN_TESTERS      = 2;
    static constexpr size_t NUMBER_OF_ADDRESS_LISTS    = 2;

    using EthernetTesters           = std::array<LogicalAddress, NUMBER_OF_ETHERNET_TESTERS>;
    using CanTesters                = std::array<LogicalAddress, NUMBER_OF_CAN_TESTERS>;
    using LogicalAddressConverterUT = LogicalAddressConverter<NUMBER_OF_ADDRESS_LISTS>;

    static constexpr EthernetTesters TESTER_ADDRESS_RANGE_ETHERNET
        = {{{0xABCDU, 0x00CDU}, {0x1234U, 0x0012U}, {0xF1F2U, 0x00FFU}}};
    static constexpr CanTesters TESTER_ADDRESS_RANGE_CAN
        = {{{0xA11DU, 0x0010U}, {0xDF01U, 0x0001U}}};

    static bool isFunctionalAddress(uint16_t address);

    static bool isFunctionallyAddressed(TransportMessage const& message);

    static bool isFromTester(TransportMessage const& message);
};

inline bool TransportConfiguration::isFunctionalAddress(uint16_t const address)
{
    return (FUNCTIONAL_ALL_ISO14229 == address);
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

} // namespace transport
