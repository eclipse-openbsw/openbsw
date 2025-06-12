// Copyright 2025 Accenture.

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
