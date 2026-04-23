// Copyright 2026 Accenture.

#include "transport/TransportConfiguration.h"

#include <etl/array.h>
#include <etl/span.h>

namespace transport
{
constexpr TransportConfiguration::EthernetTesters const
    TransportConfiguration::TESTER_ADDRESS_RANGE_ETHERNET;
constexpr TransportConfiguration::CanTesters const TransportConfiguration::TESTER_ADDRESS_RANGE_CAN;
template<>
etl::array<::etl::span<LogicalAddress const>, TransportConfiguration::NUMBER_OF_ADDRESS_LISTS> const
    TransportConfiguration::LogicalAddressConverterUT::TESTER_ADDRESS_LISTS
    = {TransportConfiguration::TESTER_ADDRESS_RANGE_ETHERNET,
       TransportConfiguration::TESTER_ADDRESS_RANGE_CAN};
} // namespace transport
