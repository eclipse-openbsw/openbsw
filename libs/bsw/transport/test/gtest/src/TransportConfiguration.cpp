// Copyright 2024 Accenture.

#include "transport/TransportConfiguration.h"

#include <etl/array.h>
#include <etl/span.h>

namespace transport
{
constexpr TransportConfiguration::InternalTesters const
    TransportConfiguration::INTERNAL_ADDRESS_RANGE;
constexpr TransportConfiguration::ExternalTesters const
    TransportConfiguration::EXTERNAL_ADDRESS_RANGE;
template<>
etl::array<::etl::span<LogicalAddress const>, TransportConfiguration::NUMBER_OF_ADDRESS_LISTS> const
    TransportConfiguration::LogicalAddressConverterUT::TESTER_ADDRESS_LISTS
    = {TransportConfiguration::INTERNAL_ADDRESS_RANGE,
       TransportConfiguration::EXTERNAL_ADDRESS_RANGE};
} // namespace transport
