/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#ifndef SOMEIP_PORT_RANGE_FUNCTION_H
#define SOMEIP_PORT_RANGE_FUNCTION_H

#include "someip/SomeIpConstants.h"

#include <cstdint>

#include <etl/tuple.h>

etl::tuple<uint16_t, ::someip::PortRangeReturnCode> computeNextLocalPort(uint16_t, uint16_t);

#endif /* SOMEIP_PORT_RANGE_FUNCTION_H */
