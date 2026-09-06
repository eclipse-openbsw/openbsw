/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "eeprom/EepromDriver.h"

namespace eeprom
{

static constexpr EepromConfiguration EEPROM_CONFIG
    = {0x14000000UL, // Base FlexRAM address
       4 * 1024,     // Total size of FlexRAM area
       0,            // Protected area size
       false,        // Load EEPROM during reset
       2,            // EEPROM Data Set size code 2 - 4KB Data Set size
       4};           // DEPART

} // namespace eeprom
