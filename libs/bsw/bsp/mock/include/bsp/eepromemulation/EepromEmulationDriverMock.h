// Copyright 2024 Accenture.

#ifndef GUARD_910913D6_00E7_4914_866C_328627B10675
#define GUARD_910913D6_00E7_4914_866C_328627B10675

#include "bsp/eepromemulation/IEepromEmulationDriver.h"

#include <gmock/gmock.h>

namespace eepromemulation
{
class EepromEmulationDriverMock : public IEepromEmulationDriver
{
public:
    MOCK_METHOD1(init, EepromEmulationReturnCode(bool));
    MOCK_METHOD3(read, EepromEmulationReturnCode(uint16_t, uint8_t*, uint16_t&));
    MOCK_METHOD3(write, EepromEmulationReturnCode(uint16_t, uint8_t const*, uint16_t));
    MOCK_METHOD6(
        write2,
        EepromEmulationReturnCode(
            uint16_t, uint16_t, uint8_t const*, uint8_t const*, uint16_t, uint16_t));
};

} // namespace eepromemulation

#endif /* GUARD_910913D6_00E7_4914_866C_328627B10675 */
