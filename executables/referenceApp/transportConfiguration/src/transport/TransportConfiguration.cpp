/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "transport/TransportConfiguration.h"

namespace transport
{
constexpr TransportConfiguration::TesterAddresses const
    TransportConfiguration::TESTER_ADDRESS_RANGE;

template<>
etl::array<::etl::span<LogicalAddress const>, TransportConfiguration::NUMBER_OF_ADDRESS_LISTS> const
    TransportConfiguration::LogicalAddressConverterGateway::TESTER_ADDRESS_LISTS
    = {TransportConfiguration::TESTER_ADDRESS_RANGE};
} // namespace transport
