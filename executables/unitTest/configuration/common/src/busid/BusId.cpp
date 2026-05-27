/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "busid/BusId.h"

namespace common
{
namespace busid
{

#define BUS_ID_NAME(BUS) \
    case ::busid::BUS: return #BUS

char const* BusIdTraits::getName(uint8_t index)
{
    switch (index)
    {
        BUS_ID_NAME(SELFDIAG);
        BUS_ID_NAME(CAN_0);
        BUS_ID_NAME(ETH_0);
        BUS_ID_NAME(ETH_1);
        default: return "INVALID";
    }
}

} // namespace busid
} // namespace common
