// Copyright 2024 Accenture.

#pragma once

#include "bsp/can/canTransceiver/CanPhy.h"

#include <gmock/gmock.h>

namespace bios
{
class CanPhyMock : public CanPhy
{
public:
    MOCK_METHOD(void, init, (uint32_t), (override));
    MOCK_METHOD(bool, setMode, (Mode, uint32_t), (override));
    MOCK_METHOD(Mode, getMode, (uint32_t), (override));
    MOCK_METHOD(ErrorCode, getPhyErrorStatus, (uint32_t), (override));
};

} // namespace bios
