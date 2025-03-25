// Copyright 2024 Accenture.

#pragma once

#include "estd/singleton.h"
#include "interrupts/disableEnableAllInterrupts.h"

#include <gmock/gmock.h>

namespace interrupts
{
class DisableEnableAllInterruptsMock : public ::estd::singleton<DisableEnableAllInterruptsMock>
{
public:
    DisableEnableAllInterruptsMock() : ::estd::singleton<DisableEnableAllInterruptsMock>(*this)
    {
        EXPECT_EQ(disableAllInterruptsCount, enableAllInterruptsCount);
        disableAllInterruptsCount = 0U;
        enableAllInterruptsCount  = 0U;
    }

    ~DisableEnableAllInterruptsMock()
    {
        EXPECT_EQ(disableAllInterruptsCount, 0U);
        EXPECT_EQ(enableAllInterruptsCount, 0U);
    }

    MOCK_METHOD0(disableAllInterrupts, void());
    MOCK_METHOD0(enableAllInterrupts, void());
    MOCK_METHOD0(areInterruptsDisabled, bool());
    MOCK_METHOD0(areInterruptsEnabled, bool());

    static uint32_t disableAllInterruptsCount;
    static uint32_t enableAllInterruptsCount;
};

} /* namespace interrupts */

