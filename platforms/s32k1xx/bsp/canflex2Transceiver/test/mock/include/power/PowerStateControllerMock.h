// Copyright 2024 Accenture.

#pragma once

#include <bsp/power/IEcuPowerStateController.h>

#include <gmock/gmock.h>

using namespace ::bios;

namespace power
{
class PowerStateControllerMock : public IEcuPowerStateController
{
public:
    MOCK_METHOD(void, startPreSleep, (), (override));
    MOCK_METHOD(uint32_t, powerDown, (uint8_t mode, tCheckWakeupDelegate delegate), (override));
    MOCK_METHOD(uint32_t, powerDown, (uint8_t mode), (override));
    MOCK_METHOD(void, fullPowerUp, (), (override));
    MOCK_METHOD(
        void,
        setWakeupSourceMonitoring,
        (uint32_t source, bool active, bool fallingEdge),
        (override));
    MOCK_METHOD(void, clearWakeupSourceMonitoring, (uint32_t source), (override));
    MOCK_METHOD(bool, setWakeupDelegate, (tCheckWakeupDelegate & delegate), (override));
    MOCK_METHOD(uint32_t, getWakeupSource, (), (override));
};

} // namespace power
