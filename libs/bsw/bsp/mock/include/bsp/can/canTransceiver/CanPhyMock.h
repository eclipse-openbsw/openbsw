// Copyright 2024 Accenture.

#pragma once

#include "bsp/can/canTransceiver/CanPhy.h"

#include <gmock/gmock.h>

namespace bios
{
class CanPhyMock : public CanPhy
{
public:
    MOCK_METHOD1(init, void(uint32_t));
    MOCK_METHOD2(setMode, bool(Mode, uint32_t));
    MOCK_METHOD1(getMode, Mode(uint32_t));
    MOCK_METHOD1(getPhyErrorStatus, ErrorCode(uint32_t));
};

} // namespace bios
