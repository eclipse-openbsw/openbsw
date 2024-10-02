// Copyright 2024 Accenture.

#ifndef GUARD_1D67FFBD_1C71_42D1_AB52_40EB954F7F12
#define GUARD_1D67FFBD_1C71_42D1_AB52_40EB954F7F12

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

#endif // GUARD_1D67FFBD_1C71_42D1_AB52_40EB954F7F12
