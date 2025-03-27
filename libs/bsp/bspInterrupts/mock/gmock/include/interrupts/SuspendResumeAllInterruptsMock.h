// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "estd/singleton.h"
#include "interrupts/suspendResumeAllInterrupts.h"

#include <gmock/gmock.h>

namespace interrupts
{

class SuspendResumeAllInterruptsMock : public ::estd::singleton<SuspendResumeAllInterruptsMock>
{
public:
    SuspendResumeAllInterruptsMock() : ::estd::singleton<SuspendResumeAllInterruptsMock>(*this)
    {
        if (resumeAllInterruptsCount != 0U)
        {
            if (getOldIntEnabledStatusValueAndSuspendAllInterruptsCount != 0U)
            {
                EXPECT_EQ(
                    getOldIntEnabledStatusValueAndSuspendAllInterruptsCount,
                    resumeAllInterruptsCount);
            }
            else
            {
                EXPECT_EQ(
                    getMachineStateRegisterValueAndSuspendAllInterruptsCount,
                    resumeAllInterruptsCount);
            }
        }
        getOldIntEnabledStatusValueAndSuspendAllInterruptsCount  = 0U;
        getMachineStateRegisterValueAndSuspendAllInterruptsCount = 0U;
        resumeAllInterruptsCount                                 = 0U;
    }

    ~SuspendResumeAllInterruptsMock()
    {
        EXPECT_EQ(getOldIntEnabledStatusValueAndSuspendAllInterruptsCount, 0U);
        EXPECT_EQ(getMachineStateRegisterValueAndSuspendAllInterruptsCount, 0U);
        EXPECT_EQ(resumeAllInterruptsCount, 0U);
    }

    MOCK_METHOD0(
        getOldIntEnabledStatusValueAndSuspendAllInterrupts, OldIntEnabledStatusValueType());
    MOCK_METHOD0(getMachineStateRegisterValueAndSuspendAllInterrupts, uint32_t());
    MOCK_METHOD1(resumeAllInterrupts, void(OldIntEnabledStatusValueType));

    static uint32_t getOldIntEnabledStatusValueAndSuspendAllInterruptsCount;
    static uint32_t getMachineStateRegisterValueAndSuspendAllInterruptsCount;
    static uint32_t resumeAllInterruptsCount;
};

} /* namespace interrupts */
