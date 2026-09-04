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

#include "someip/SdEndpoint.h"

namespace someip
{
class SdOptions;

struct SdOptionParser
{
    static SdEndpoint const INVALID_ENDPOINT;

    static SdEndpoint
    parseIpEndpointOption(SdOptions const& options, uint8_t& numUdpOptions, uint8_t& numTcpOptions);

    static SdEndpoint parseIpMulticastOption(
        SdOptions const& options, uint8_t& numUdpOptions, uint8_t& numTcpOptions);
};

} // namespace someip
