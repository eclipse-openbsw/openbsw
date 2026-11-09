/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/TcpProxy.h"

#include <etl/tuple.h>

// cpp2someip leaves the choice of local port to the application; the tests take the requested one.
etl::tuple<uint16_t, ::someip::PortRangeReturnCode>
computeNextLocalPort(uint16_t const requestedPort, uint16_t const)
{
    return etl::make_tuple(requestedPort, ::someip::PortRangeReturnCode::OK);
}
