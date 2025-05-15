// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "interrupts/suspendResumeAllInterrupts.h"

#include <etl/singleton_base.h>

#include <gmock/gmock.h>

namespace interrupts
{

class SuspendResumeAllInterruptsMock : public ::etl::singleton_base<SuspendResumeAllInterruptsMock>
{
public:
    SuspendResumeAllInterruptsMock() : ::etl::singleton_base<SuspendResumeAllInterruptsMock>(*this)
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

    MOCK_METHOD(
        OldIntEnabledStatusValueType, getOldIntEnabledStatusValueAndSuspendAllInterrupts, ());
    MOCK_METHOD(uint32_t, getMachineStateRegisterValueAndSuspendAllInterrupts, ());
    MOCK_METHOD(void, resumeAllInterrupts, (OldIntEnabledStatusValueType));

    static uint32_t getOldIntEnabledStatusValueAndSuspendAllInterruptsCount;
    static uint32_t getMachineStateRegisterValueAndSuspendAllInterruptsCount;
    static uint32_t resumeAllInterruptsCount;
};

} /* namespace interrupts */
