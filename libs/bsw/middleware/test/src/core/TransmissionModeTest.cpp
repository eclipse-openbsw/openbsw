/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <gtest/gtest.h>

#include "middleware/core/Transmission.h"
#include "middleware/core/TransmissionMode.h"

namespace middleware::core::test
{

constexpr uint16_t ARBITRARY_CYCLIC_TIME_MS  = 10U;
constexpr uint8_t ARBITRARY_DEBOUNCE_TIME_MS = 5U;
constexpr uint8_t NO_REPETITION_INTERVAL_MS  = 0U;
constexpr uint8_t NO_REPETITIONS             = 0U;

struct SendRequestingStrategy
{
    static inline uint8_t executeCalls{};

    bool execute()
    {
        ++executeCalls;
        return true;
    }
};

struct NoSendRequestingStrategy
{
    static inline uint8_t executeCalls{};

    bool execute()
    {
        ++executeCalls;
        return false;
    }
};

TEST(TransmissionModeTest, ExecuteReturnsTrueAtStartWhenCyclicTransmissionIsPresent)
{
    TransmissionMode<CyclicTransmission<ARBITRARY_CYCLIC_TIME_MS>, NoTransmission> mode;

    EXPECT_TRUE(mode.execute());
}

TEST(TransmissionModeTest, ExecuteReturnsFalseAtStartForEventTransmission)
{
    TransmissionMode<
        EventTransmission<ARBITRARY_DEBOUNCE_TIME_MS, NO_REPETITION_INTERVAL_MS, NO_REPETITIONS>>
        mode;

    EXPECT_FALSE(mode.execute());
}

TEST(TransmissionModeTest, ExecuteReturnsTrueAtStartWhenOnlyCyclicRequestsSend)
{
    TransmissionMode<
        CyclicTransmission<ARBITRARY_CYCLIC_TIME_MS>,
        EventTransmission<ARBITRARY_DEBOUNCE_TIME_MS, NO_REPETITION_INTERVAL_MS, NO_REPETITIONS>,
        NoTransmission>
        mode;
    EXPECT_TRUE(mode.execute());
}

TEST(TransmissionModeTest, ExecuteRunsAllStrategiesAfterOneRequestsSend)
{
    TransmissionMode<SendRequestingStrategy, NoSendRequestingStrategy> mode;
    SendRequestingStrategy::executeCalls   = 0U;
    NoSendRequestingStrategy::executeCalls = 0U;

    EXPECT_TRUE(mode.execute());
    EXPECT_EQ(SendRequestingStrategy::executeCalls, 1U);
    EXPECT_EQ(NoSendRequestingStrategy::executeCalls, 1U);
}

TEST(TransmissionModeTest, TriggerEventDefersUntilExecuteWhenEventTransmissionIsPresent)
{
    TransmissionMode<
        EventTransmission<ARBITRARY_DEBOUNCE_TIME_MS, NO_REPETITION_INTERVAL_MS, NO_REPETITIONS>>
        mode;

    EXPECT_FALSE(mode.triggerEvent());
    EXPECT_TRUE(mode.execute());
}

TEST(TransmissionModeTest, TriggerEventReturnsFalseAtStartWhenOnlyNoTransmissionIsPresent)
{
    TransmissionMode<NoTransmission> mode;

    EXPECT_FALSE(mode.triggerEvent());
}

TEST(TransmissionModeTest, TriggerEventDefersWhenCyclicAndEventStrategiesArePresent)
{
    TransmissionMode<
        CyclicTransmission<ARBITRARY_CYCLIC_TIME_MS>,
        EventTransmission<ARBITRARY_DEBOUNCE_TIME_MS, NO_REPETITION_INTERVAL_MS, NO_REPETITIONS>,
        NoTransmission>
        mode;

    EXPECT_FALSE(mode.triggerEvent());
    EXPECT_TRUE(mode.execute());
    EXPECT_FALSE(mode.execute());
}

} // namespace middleware::core::test
