// Copyright 2024 Accenture.

#ifndef GUARD_AE103AC8_B8E9_4574_8FEB_8320387EADFB
#define GUARD_AE103AC8_B8E9_4574_8FEB_8320387EADFB

#include "bsp/eeprom/IEepromDriver.h"

#include <gmock/gmock.h>

namespace eeprom
{
class EepromDriverMock : public IEepromDriver
{
public:
    MOCK_METHOD0(init, bsp::BspReturnCode());
    MOCK_METHOD3(write, bsp::BspReturnCode(uint32_t, uint8_t const*, uint32_t));
    MOCK_METHOD3(read, bsp::BspReturnCode(uint32_t, uint8_t*, uint32_t));
};

} // namespace eeprom

#endif /* GUARD_AE103AC8_B8E9_4574_8FEB_8320387EADFB */
