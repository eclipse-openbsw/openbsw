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

#include "middleware/core/TransmissionModeManager.h"
#include "mock/ModeMock.h"

namespace middleware::core::test
{

using Mode1 = ModeMockAdapter<1U>;
using Mode2 = ModeMockAdapter<2U>;

class TransmissionModeManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Mode1::setMock(&mode1Mock_);
        Mode2::setMock(&mode2Mock_);
    }

    void TearDown() override
    {
        Mode1::unsetMock();
        Mode2::unsetMock();
    }

    ::testing::StrictMock<ModeMock> mode1Mock_{};
    ::testing::StrictMock<ModeMock> mode2Mock_{};
    TransmissionModeManager<Mode1, Mode2> manager_{};
};

TEST_F(TransmissionModeManagerTest, CyclicUsesMode1ByDefault)
{
    mode1Mock_.EXPECT_EXECUTE(true);
    mode2Mock_.EXPECT_NO_EXECUTE();

    EXPECT_TRUE(manager_.cyclic());
}

TEST_F(TransmissionModeManagerTest, CyclicUsesMode2WhenActiveConditionIsTrue)
{
    mode1Mock_.EXPECT_CANCEL_PENDING_TRANSMISSIONS();
    mode1Mock_.EXPECT_NO_EXECUTE();
    mode2Mock_.EXPECT_RESTART_CYCLIC_TIMER(1);
    mode2Mock_.EXPECT_EXECUTE(true);

    EXPECT_TRUE(manager_.setActiveCondition(true));

    EXPECT_TRUE(manager_.cyclic());
}

TEST_F(TransmissionModeManagerTest, TriggerEventUsesMode1ByDefault)
{
    mode1Mock_.EXPECT_TRIGGER_EVENT(true);
    mode2Mock_.EXPECT_NO_TRIGGER_EVENT();

    EXPECT_TRUE(manager_.triggerEvent());
}

TEST_F(TransmissionModeManagerTest, TriggerEventUsesMode2WhenActiveConditionIsTrue)
{
    mode1Mock_.EXPECT_CANCEL_PENDING_TRANSMISSIONS();
    mode2Mock_.EXPECT_TRIGGER_EVENT(true);
    mode2Mock_.EXPECT_RESTART_CYCLIC_TIMER(1);
    mode1Mock_.EXPECT_NO_TRIGGER_EVENT();

    EXPECT_TRUE(manager_.setActiveCondition(true));

    EXPECT_TRUE(manager_.triggerEvent());
}

TEST_F(TransmissionModeManagerTest, CyclicMode1ActivePropagatesAggregatedCount)
{
    mode1Mock_.EXPECT_EXECUTE(true);
    mode2Mock_.EXPECT_NO_EXECUTE();

    EXPECT_TRUE(manager_.cyclic());
}

TEST_F(TransmissionModeManagerTest, CyclicMode2ActivePropagatesAggregatedCount)
{
    mode1Mock_.EXPECT_CANCEL_PENDING_TRANSMISSIONS();
    mode1Mock_.EXPECT_NO_EXECUTE();
    mode2Mock_.EXPECT_RESTART_CYCLIC_TIMER(1);
    mode2Mock_.EXPECT_EXECUTE(true);

    EXPECT_TRUE(manager_.setActiveCondition(true));

    EXPECT_TRUE(manager_.cyclic());
}

TEST_F(TransmissionModeManagerTest, TriggerEventMode1ActivePropagatesModeRequest)
{
    mode1Mock_.EXPECT_TRIGGER_EVENT(true);
    mode2Mock_.EXPECT_NO_TRIGGER_EVENT();

    EXPECT_TRUE(manager_.triggerEvent());
}

TEST_F(TransmissionModeManagerTest, TriggerEventMode2ActivePropagatesModeRequest)
{
    mode1Mock_.EXPECT_CANCEL_PENDING_TRANSMISSIONS();
    mode1Mock_.EXPECT_NO_TRIGGER_EVENT();
    mode2Mock_.EXPECT_RESTART_CYCLIC_TIMER(1);
    mode2Mock_.EXPECT_TRIGGER_EVENT(true);

    EXPECT_TRUE(manager_.setActiveCondition(true));

    EXPECT_TRUE(manager_.triggerEvent());
}

TEST_F(
    TransmissionModeManagerTest, SetActiveConditionFalseWhenAlreadyMode1DoesNotRequestImmediateSend)
{
    mode1Mock_.EXPECT_CANCEL_PENDING_TRANSMISSIONS(0U);
    mode2Mock_.EXPECT_CANCEL_PENDING_TRANSMISSIONS(0U);
    mode1Mock_.EXPECT_EXECUTE(true);
    mode2Mock_.EXPECT_RESTART_CYCLIC_TIMER(0);
    mode2Mock_.EXPECT_NO_EXECUTE();

    EXPECT_FALSE(manager_.setActiveCondition(false));

    EXPECT_TRUE(manager_.cyclic());
}

TEST_F(TransmissionModeManagerTest, SetActiveConditionTrueTwiceRequestsImmediateSendOnlyOnce)
{
    mode1Mock_.EXPECT_CANCEL_PENDING_TRANSMISSIONS();
    mode2Mock_.EXPECT_CANCEL_PENDING_TRANSMISSIONS(0U);
    mode2Mock_.EXPECT_RESTART_CYCLIC_TIMER(1);
    mode1Mock_.EXPECT_NO_EXECUTE();
    mode2Mock_.EXPECT_EXECUTE(true);

    EXPECT_TRUE(manager_.setActiveCondition(true));
    EXPECT_FALSE(manager_.setActiveCondition(true));

    EXPECT_TRUE(manager_.cyclic());
}

TEST_F(TransmissionModeManagerTest, SetActiveConditionCanReturnFalseAndStillSwitchToMode2)
{
    mode1Mock_.EXPECT_CANCEL_PENDING_TRANSMISSIONS();
    mode2Mock_.EXPECT_RESTART_CYCLIC_TIMER(1, false); // no cyclic strategy in active mode
    mode1Mock_.EXPECT_NO_EXECUTE();
    mode2Mock_.EXPECT_EXECUTE(true);

    EXPECT_FALSE(manager_.setActiveCondition(true));
    EXPECT_TRUE(manager_.cyclic());
}

TEST_F(TransmissionModeManagerTest, SetActiveConditionCanReturnFalseAndStillSwitchBackToMode1)
{
    mode1Mock_.EXPECT_CANCEL_PENDING_TRANSMISSIONS();
    mode2Mock_.EXPECT_CANCEL_PENDING_TRANSMISSIONS();
    mode2Mock_.EXPECT_RESTART_CYCLIC_TIMER(1, true);
    mode1Mock_.EXPECT_RESTART_CYCLIC_TIMER(1, false); // no cyclic strategy in Mode1
    mode2Mock_.EXPECT_NO_EXECUTE();
    mode1Mock_.EXPECT_EXECUTE(true);

    EXPECT_TRUE(manager_.setActiveCondition(true));
    EXPECT_FALSE(manager_.setActiveCondition(false));
    EXPECT_TRUE(manager_.cyclic());
}

TEST_F(TransmissionModeManagerTest, SwitchingBackToMode1ForcesRestartTimerOnMode1)
{
    {
        ::testing::InSequence sequence;
        mode1Mock_.EXPECT_CANCEL_PENDING_TRANSMISSIONS();
        mode2Mock_.EXPECT_RESTART_CYCLIC_TIMER(1);
        mode2Mock_.EXPECT_CANCEL_PENDING_TRANSMISSIONS();
        mode1Mock_.EXPECT_RESTART_CYCLIC_TIMER(1);
    }

    mode1Mock_.EXPECT_EXECUTE(true);
    mode2Mock_.EXPECT_NO_EXECUTE();

    EXPECT_TRUE(manager_.setActiveCondition(true));
    EXPECT_TRUE(manager_.setActiveCondition(false));

    EXPECT_TRUE(manager_.cyclic());
}

TEST_F(TransmissionModeManagerTest, TriggerEventMode1ActivePropagatesFalse)
{
    mode1Mock_.EXPECT_TRIGGER_EVENT(false);
    mode2Mock_.EXPECT_NO_TRIGGER_EVENT();

    EXPECT_FALSE(manager_.triggerEvent());
}

TEST_F(TransmissionModeManagerTest, TriggerEventMode2ActivePropagatesFalse)
{
    mode1Mock_.EXPECT_CANCEL_PENDING_TRANSMISSIONS();
    mode2Mock_.EXPECT_RESTART_CYCLIC_TIMER(1);
    mode1Mock_.EXPECT_NO_TRIGGER_EVENT();
    mode2Mock_.EXPECT_TRIGGER_EVENT(false);

    EXPECT_TRUE(manager_.setActiveCondition(true));

    EXPECT_FALSE(manager_.triggerEvent());
}

TEST_F(
    TransmissionModeManagerTest,
    SetActiveConditionCancelsOutgoingModeAndRestartsIncomingCyclicTimer)
{
    ::testing::InSequence sequence;
    mode1Mock_.EXPECT_CANCEL_PENDING_TRANSMISSIONS();
    mode2Mock_.EXPECT_CANCEL_PENDING_TRANSMISSIONS(0U);
    mode2Mock_.EXPECT_RESTART_CYCLIC_TIMER(1);
    EXPECT_TRUE(manager_.setActiveCondition(true));
}

} // namespace middleware::core::test
