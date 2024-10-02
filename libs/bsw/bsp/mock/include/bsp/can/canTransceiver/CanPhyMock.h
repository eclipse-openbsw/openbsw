// Copyright 2024 Accenture.

#ifndef GUARD_307BC172_E810_489A_B759_A64C060F01DF
#define GUARD_307BC172_E810_489A_B759_A64C060F01DF

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

#endif /* GUARD_307BC172_E810_489A_B759_A64C060F01DF */
