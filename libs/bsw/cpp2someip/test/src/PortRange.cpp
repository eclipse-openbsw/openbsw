/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "PortRange.h"

#include <etl/tuple.h>

etl::tuple<uint16_t, ::someip::PortRangeReturnCode>
computeNextLocalPort(uint16_t const requestedPort, uint16_t const)
{
    return etl::make_tuple(requestedPort, ::someip::PortRangeReturnCode::OK);
}
