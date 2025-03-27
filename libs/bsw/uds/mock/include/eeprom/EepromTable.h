// Copyright 2024 Accenture.

#pragma once

#include <platform/estdint.h>

namespace eeprom
{
class BOOTCONFIG_SECTION_OFFSET
{
public:
    static uint8_t const SESSION                  = 0;
    static uint8_t const PROGRAMMING_DEPENDENCIES = 1;
    static uint8_t const LIFECYCLE_MODE           = 2;
};

class SVK_STATUS_SECTION_OFFSET
{
public:
    static uint8_t const OFFSET_STATUS_SVK       = 0;
    static uint8_t const PROGRAMMING_COUNTER_MAX = 5;
};

} // namespace eeprom
