// Copyright 2024 Accenture.

#pragma once

#include "bsp/power/IEcuPowerStateController.h"

#include <gmock/gmock.h>

using namespace ::bios;

namespace power
{
class EcuPowerStateControllerMock : public IEcuPowerStateController
{
public:
    MOCK_METHOD(void, startPreSleep, ());
    MOCK_METHOD(uint32_t, powerDown, (uint8_t mode, tCheckWakeupDelegate delegate));
    MOCK_METHOD(uint32_t, powerDown, (uint8_t mode));
    MOCK_METHOD(void, fullPowerUp, ());
    MOCK_METHOD(void, setWakeupSourceMonitoring, (uint32_t source, bool active, bool fallingEdge));
    MOCK_METHOD(void, clearWakeupSourceMonitoring, (uint32_t source));
    MOCK_METHOD(bool, setWakeupDelegate, (tCheckWakeupDelegate & delegate));
    MOCK_METHOD(uint32_t, getWakeupSource, (void));
};

} // namespace power
