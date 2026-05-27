/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "common/busid/BusId.h"

#include <cstdint>

namespace busid
{
static constexpr uint8_t SELFDIAG = 1;
static constexpr uint8_t CAN_0    = 2;
static constexpr uint8_t ETH_0    = 3;
static constexpr uint8_t ETH_1    = 4;
static constexpr uint8_t LAST_BUS = ETH_1;

} // namespace busid
