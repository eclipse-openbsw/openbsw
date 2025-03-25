// Copyright 2024 Accenture.

#pragma once

#include "platform/estdint.h"

namespace uds
{
class RoutineControlOptionParser
{
public:
    static uint8_t getLogicalBlockNumberLength(uint8_t lengthFormatIdentifier);
    static uint8_t getMemoryAddressLength(uint8_t lengthFormatIdentifier);
    static uint8_t getMemorySizeLength(uint8_t lengthFormatIdentifier);
    static uint32_t parseParameter(uint8_t const* buffer, uint8_t length);
};

} // namespace uds

