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
#include "middleware/core/TransmissionModeManager.h"

namespace middleware::core::test
{

class EventTransmissionTest : public ::testing::Test
{
protected:
    static constexpr uint16_t DEBOUNCE_TICKS{5U};
    static constexpr uint16_t OBSERVATION_TICKS{10U};
};

TEST_F(EventTransmissionTest, DefersAndCoalescesEventsUntilDebounceExpires)
{
    EventTransmission<DEBOUNCE_TICKS, 0U, 0U> transmission;
    EXPECT_FALSE(transmission.trigger());
    EXPECT_TRUE(transmission.execute());
    EXPECT_FALSE(transmission.trigger());
    EXPECT_FALSE(transmission.trigger());
    for (uint8_t tick = 0U; tick < DEBOUNCE_TICKS - 1U; ++tick)
    {
        EXPECT_FALSE(transmission.execute());
    }
    EXPECT_TRUE(transmission.execute());
    for (uint8_t tick = 0U; tick < OBSERVATION_TICKS; ++tick)
    {
        EXPECT_FALSE(transmission.execute());
    }
}

TEST_F(EventTransmissionTest, ZeroDebounceStillDefersAndCoalescesUntilExecute)
{
    EventTransmission<0U, 0U, 0U> transmission;
    EXPECT_FALSE(transmission.execute());
    EXPECT_FALSE(transmission.trigger());
    EXPECT_FALSE(transmission.trigger());
    EXPECT_TRUE(transmission.execute());
    EXPECT_FALSE(transmission.execute());
    EXPECT_FALSE(transmission.trigger());
    EXPECT_TRUE(transmission.execute());
}

TEST_F(EventTransmissionTest, DeferredEventStartsNewRepetitionsAtItsSendTick)
{
    EventTransmission<DEBOUNCE_TICKS, 2U> transmission;
    EXPECT_FALSE(transmission.trigger());
    ASSERT_TRUE(transmission.execute());
    EXPECT_FALSE(transmission.trigger());
    EXPECT_FALSE(transmission.execute());
    EXPECT_TRUE(transmission.execute());
    EXPECT_FALSE(transmission.execute());
    EXPECT_TRUE(transmission.execute());
    EXPECT_TRUE(transmission.execute());
    EXPECT_FALSE(transmission.execute());
    EXPECT_TRUE(transmission.execute());
    EXPECT_FALSE(transmission.execute());
    EXPECT_TRUE(transmission.execute());
    EXPECT_FALSE(transmission.execute());
}

TEST_F(EventTransmissionTest, NewEventAndDueRepetitionCoalesceIntoOneNewSequence)
{
    EventTransmission<3U, 3U, 2U> transmission;
    EXPECT_FALSE(transmission.trigger());
    ASSERT_TRUE(transmission.execute());
    EXPECT_FALSE(transmission.trigger());
    EXPECT_FALSE(transmission.execute());
    EXPECT_FALSE(transmission.execute());
    EXPECT_TRUE(transmission.execute());
    for (uint8_t repetition = 0U; repetition < 2U; ++repetition)
    {
        EXPECT_FALSE(transmission.execute());
        EXPECT_FALSE(transmission.execute());
        EXPECT_TRUE(transmission.execute());
    }
    EXPECT_FALSE(transmission.execute());
}

TEST_F(EventTransmissionTest, ModeSwitchCancelsEventsAndRepetitions)
{
    using EventMode = TransmissionMode<EventTransmission<DEBOUNCE_TICKS, 3U, 2U>>;
    TransmissionModeManager<EventMode> manager;
    EXPECT_FALSE(manager.triggerEvent());
    EXPECT_FALSE(manager.setActiveCondition(true));
    EXPECT_FALSE(manager.cyclic());
    EXPECT_FALSE(manager.setActiveCondition(false));
    EXPECT_FALSE(manager.cyclic());

    EXPECT_FALSE(manager.triggerEvent());
    ASSERT_TRUE(manager.cyclic());
    EXPECT_FALSE(manager.triggerEvent());
    EXPECT_FALSE(manager.setActiveCondition(true));
    EXPECT_FALSE(manager.setActiveCondition(false));
    for (uint8_t tick = 0U; tick < OBSERVATION_TICKS; ++tick)
    {
        EXPECT_FALSE(manager.cyclic());
    }
    EXPECT_FALSE(manager.triggerEvent());
    EXPECT_TRUE(manager.cyclic());
}

TEST_F(EventTransmissionTest, SwitchingToEmptyModeCancelsPendingCyclicModeEntry)
{
    TransmissionModeManager<TransmissionMode<CyclicTransmission<OBSERVATION_TICKS>>> manager;
    EXPECT_FALSE(manager.setActiveCondition(true));
    EXPECT_TRUE(manager.setActiveCondition(false));
    EXPECT_FALSE(manager.setActiveCondition(true));
    EXPECT_FALSE(manager.cyclic());
}

TEST_F(EventTransmissionTest, UnchangedModeConditionPreservesPendingEvent)
{
    TransmissionModeManager<TransmissionMode<EventTransmission<DEBOUNCE_TICKS, 0U, 0U>>> manager;
    EXPECT_FALSE(manager.triggerEvent());
    EXPECT_FALSE(manager.setActiveCondition(false));
    ASSERT_TRUE(manager.cyclic());
    EXPECT_FALSE(manager.triggerEvent());
    EXPECT_FALSE(manager.setActiveCondition(false));
    for (uint8_t tick = 0U; tick < DEBOUNCE_TICKS - 1U; ++tick)
    {
        EXPECT_FALSE(manager.cyclic());
    }
    EXPECT_TRUE(manager.cyclic());
}

TEST_F(EventTransmissionTest, ModeEntryNotificationAdvancesCyclicTimerAndIsConsumedOnce)
{
    TransmissionModeManager<TransmissionMode<CyclicTransmission<DEBOUNCE_TICKS>>> manager;
    EXPECT_FALSE(manager.setActiveCondition(true));
    EXPECT_TRUE(manager.setActiveCondition(false));
    EXPECT_FALSE(manager.setActiveCondition(false));
    EXPECT_TRUE(manager.cyclic());
    for (uint8_t tick = 0U; tick < 3U; ++tick)
    {
        EXPECT_FALSE(manager.cyclic());
    }
    EXPECT_TRUE(manager.cyclic());
    EXPECT_FALSE(manager.cyclic());
}

namespace
{

/**
 * \brief Helper function to count the number of send requests from cyclic() over a number of ticks.
 * \tparam Transmitter The type of the transmitter, must have a cyclic() method.
 * \param transmitter The transmitter to test.
 * \param ticks The number of ticks to call cyclic() for.
 * \return The total number of send requests returned by cyclic() over the given number of ticks.
 */
template<typename Transmitter>
uint32_t countCyclicSendRequests(Transmitter& transmitter, uint32_t const ticks)
{
    uint32_t sendCount{0U};
    for (uint32_t i{0U}; i < ticks; ++i)
    {
        sendCount += transmitter.cyclic();
    }
    return sendCount;
}

/**
 * \brief Helper function to consume the initial cyclic transmission at t=0.
 * \tparam Transmitter The type of the transmitter, must have a cyclic() method.
 * \param transmitter The transmitter to test.
 */
template<typename Transmitter>
void consumeInitialCyclicAtT0(Transmitter& transmitter)
{
    ASSERT_EQ(countCyclicSendRequests(transmitter, 1U), 1U);
}

} // namespace

/**
 * \brief Test fixture for transmission class unit tests.
 * \see middleware_transmission_unittest.cpp
 */
class TransmissionTest : public ::testing::Test
{
protected:
    static constexpr uint8_t DEBOUNCE_TIME_MS       = 5U;
    static constexpr uint8_t REPETITION_COUNT       = 2U;
    static constexpr uint8_t REPETITION_INTERVAL_MS = 3U;
    static constexpr uint8_t CYCLE_INTERVAL_MS      = 10U;

    using EventTx  = EventTransmission<DEBOUNCE_TIME_MS, REPETITION_INTERVAL_MS, REPETITION_COUNT>;
    using CyclicTx = CyclicTransmission<CYCLE_INTERVAL_MS>;
    using Mode1    = TransmissionMode<EventTx, CyclicTx>;
    using Mode2    = TransmissionMode<NoTransmission>;
    using ModeManager = TransmissionModeManager<Mode1, Mode2>;

    ModeManager transmitter_{};
};

// START_TEST_CASE: PduTxEventTriggered
/**
 *
 * **Purpose**
 *
 * This test verifies that when an event trigger is called for a PDU, the PDU is sent.
 *
 * **Given**
 *
 * * A TransmissionModeManager with event-triggered transmission configured.
 *
 * **When**
 *
 * * triggerEvent() is called.
 *
 * **Then**
 *
 * * The trigger records a request, and cyclic() initiates the transmission.
 */
TEST_F(TransmissionTest, PduTxEventTriggered)
{
    // ACT
    bool const canSendPdu = transmitter_.triggerEvent();

    // ASSERT
    EXPECT_FALSE(canSendPdu);
    EXPECT_TRUE(transmitter_.cyclic());
}

// END_TEST_CASE: PduTxEventTriggered

// START_TEST_CASE: EventTriggerDebounceGate
/**
 *
 * **Purpose**
 *
 * This test verifies that a pending event is released only after debounce expires.
 *
 * **Given**
 *
 * * An event strategy with debounce and no repetitions configured.
 *
 * **When**
 *
 * * An event is transmitted, then another event is triggered during debounce.
 *
 * **Then**
 *
 * * The second event waits for debounce expiry without requiring another trigger.
 */
TEST_F(TransmissionTest, EventTriggerDebounceGate)
{
    EventTransmission<DEBOUNCE_TIME_MS, 0U, 0U> transmission;
    EXPECT_FALSE(transmission.trigger());
    ASSERT_TRUE(transmission.execute());
    EXPECT_FALSE(transmission.trigger());
    for (uint8_t tick = 0U; tick < DEBOUNCE_TIME_MS - 1U; ++tick)
    {
        EXPECT_FALSE(transmission.execute());
    }
    EXPECT_TRUE(transmission.execute());
}

// END_TEST_CASE: EventTriggerDebounceGate

// START_TEST_CASE: EventTriggerDebounceEtRepetitionUnaffected
/**
 *
 * **Purpose**
 *
 * This test verifies that debounce does not affect the repetitions of an event-triggered
 * transmission with repetition.
 *
 * **Given**
 *
 * * A TransmissionModeManager with event-triggered transmission with repetition configured.
 *
 * **When**
 *
 * * The initial cyclic transmission at t=0 is consumed.
 * * triggerEvent() is called and repetition intervals elapse while debounce is still active.
 *
 * **Then**
 *
 * * Repetitions fire at each repetition interval despite debounce being active.
 */
TEST_F(TransmissionTest, EventTriggerDebounceEtRepetitionUnaffected)
{
    // ARRANGE
    consumeInitialCyclicAtT0(transmitter_);
    ASSERT_FALSE(transmitter_.triggerEvent());
    ASSERT_TRUE(transmitter_.cyclic());

    // ACT
    uint32_t const firstIntervalSends
        = countCyclicSendRequests(transmitter_, REPETITION_INTERVAL_MS);
    uint32_t const secondIntervalSends
        = countCyclicSendRequests(transmitter_, REPETITION_INTERVAL_MS);

    // ASSERT
    EXPECT_EQ(firstIntervalSends, 1U);
    EXPECT_EQ(secondIntervalSends, 1U);
}

// END_TEST_CASE: EventTriggerDebounceEtRepetitionUnaffected

// START_TEST_CASE: PduTxEventTriggeredWithRepetition
/**
 *
 * **Purpose**
 *
 * This test verifies that when event-triggered transmission with repetition is configured and
 * an event trigger is accepted, the configured number of additional transmissions shall be sent
 * spaced at the repetition interval.
 *
 * **Given**
 *
 * * A TransmissionModeManager with event-triggered transmission with repetition configured.
 *
 * **When**
 *
 * * The initial cyclic transmission at t=0 is consumed.
 * * triggerEvent() is called and time advances through all repetition intervals.
 *
 * **Then**
 *
 * * Exactly REPETITION_COUNT additional transmissions are sent, and no further repetitions
 *   occur after all are exhausted.
 */
TEST_F(TransmissionTest, PduTxEventTriggeredWithRepetition)
{
    // ARRANGE
    consumeInitialCyclicAtT0(transmitter_);
    ASSERT_FALSE(transmitter_.triggerEvent());
    ASSERT_TRUE(transmitter_.cyclic());

    // ACT
    uint32_t const repetitionSends = countCyclicSendRequests(
        transmitter_, static_cast<uint32_t>(REPETITION_COUNT) * REPETITION_INTERVAL_MS);
    uint32_t const sendsAfterRepetitions
        = countCyclicSendRequests(transmitter_, REPETITION_INTERVAL_MS);

    // ASSERT
    EXPECT_EQ(repetitionSends, static_cast<uint32_t>(REPETITION_COUNT));
    EXPECT_EQ(sendsAfterRepetitions, 1U)
        << "Only the cyclic strategy should send after the repetitions are exhausted";
}

// END_TEST_CASE: PduTxEventTriggeredWithRepetition

// START_TEST_CASE: EventTriggeredDefaultRepetitionCount
/**
 *
 * **Purpose**
 *
 * This test verifies that after an initial event-triggered with repetition transmission, the
 * default repetition count of two additional transmissions shall be used, configurable at
 * integration time.
 *
 * **Given**
 *
 * * A TransmissionModeManager with REPETITION_COUNT configured to 2 at integration time.
 *
 * **When**
 *
 * * The initial cyclic transmission at t=0 is consumed.
 * * triggerEvent() is called and time advances through two repetition intervals.
 *
 * **Then**
 *
 * * Exactly 2 additional transmissions are sent.
 */
TEST_F(TransmissionTest, EventTriggeredDefaultRepetitionCount)
{
    // ARRANGE
    consumeInitialCyclicAtT0(transmitter_);
    ASSERT_FALSE(transmitter_.triggerEvent());
    ASSERT_TRUE(transmitter_.cyclic());

    // ACT
    uint32_t const sendCount = countCyclicSendRequests(transmitter_, 2U * REPETITION_INTERVAL_MS);

    // ASSERT
    EXPECT_EQ(sendCount, 2U);
}

// END_TEST_CASE: EventTriggeredDefaultRepetitionCount

// START_TEST_CASE: EventTriggeredRepetitionTimeMaintained
/**
 *
 * **Purpose**
 *
 * This test verifies that each repeated event-triggered TX request shall be issued at the
 * nominal trep interval after the previous transmission.
 *
 * **Given**
 *
 * * A TransmissionModeManager with event-triggered transmission with repetition configured.
 *
 * **When**
 *
 * * The initial cyclic transmission at t=0 is consumed.
 * * triggerEvent() is called and time advances in sub-interval increments.
 *
 * **Then**
 *
 * * Repetitions fire exactly at each REPETITION_INTERVAL_MS boundary and not before.
 */
TEST_F(TransmissionTest, EventTriggeredRepetitionTimeMaintained)
{
    // ARRANGE
    consumeInitialCyclicAtT0(transmitter_);
    ASSERT_FALSE(transmitter_.triggerEvent());
    ASSERT_TRUE(transmitter_.cyclic());

    // ACT
    uint32_t const sendsBeforeFirstBoundary
        = countCyclicSendRequests(transmitter_, REPETITION_INTERVAL_MS - 1U);
    uint32_t const sendsAtFirstBoundary = countCyclicSendRequests(transmitter_, 1U);
    uint32_t const sendsBeforeSecondBoundary
        = countCyclicSendRequests(transmitter_, REPETITION_INTERVAL_MS - 1U);
    uint32_t const sendsAtSecondBoundary = countCyclicSendRequests(transmitter_, 1U);

    // ASSERT
    EXPECT_EQ(sendsBeforeFirstBoundary, 0U);
    EXPECT_EQ(sendsAtFirstBoundary, 1U);
    EXPECT_EQ(sendsBeforeSecondBoundary, 0U);
    EXPECT_EQ(sendsAtSecondBoundary, 1U);
}

// END_TEST_CASE: EventTriggeredRepetitionTimeMaintained

// START_TEST_CASE: NoDebounceTimerStartForRepeatedTransmissions
/**
 *
 * **Purpose**
 *
 * This test verifies that the debounce timer shall not be started when an event-triggered
 * transmission with repetition is sent.
 *
 * **Given**
 *
 * * A TransmissionModeManager with event-triggered transmission with repetition and debounce
 *   configured.
 *
 * **When**
 *
 * * The initial cyclic transmission at t=0 is consumed.
 * * triggerEvent() is called, all repetitions complete, and triggerEvent() is called again.
 *
 * **Then**
 *
 * * The second event sends on cyclic() because repetitions did not restart debounce.
 */
TEST_F(TransmissionTest, NoDebounceTimerStartForRepeatedTransmissions)
{
    // ARRANGE
    consumeInitialCyclicAtT0(transmitter_);
    ASSERT_FALSE(transmitter_.triggerEvent());
    ASSERT_TRUE(transmitter_.cyclic());

    // ACT
    uint32_t const repetitionSends = countCyclicSendRequests(
        transmitter_, static_cast<uint32_t>(REPETITION_COUNT) * REPETITION_INTERVAL_MS);
    bool const canSendPdu = transmitter_.triggerEvent();

    // ASSERT
    EXPECT_EQ(repetitionSends, static_cast<uint32_t>(REPETITION_COUNT));
    EXPECT_FALSE(canSendPdu);
    EXPECT_TRUE(transmitter_.cyclic());
}

// END_TEST_CASE: NoDebounceTimerStartForRepeatedTransmissions

// START_TEST_CASE: PduTxCyclicTransmissionInitialBehavior
/**
 *
 * **Purpose**
 *
 * This test verifies that cyclic transmission does not send before the configured cyclic
 * interval elapses.
 *
 * **Given**
 *
 * * A TransmissionModeManager with cyclic transmission configured with CYCLE_INTERVAL_MS.
 *
 * **When**
 *
 * * The initial cyclic transmission at t=0 is consumed.
 * * cyclic() is called before the next interval elapses.
 *
 * **Then**
 *
 * * No transmission is requested.
 */
TEST_F(TransmissionTest, PduTxCyclicTransmissionInitialBehavior)
{
    // ARRANGE
    consumeInitialCyclicAtT0(transmitter_);

    // ACT AND ASSERT
    EXPECT_EQ(countCyclicSendRequests(transmitter_, CYCLE_INTERVAL_MS - 1U), 0U);
}

// END_TEST_CASE: PduTxCyclicTransmissionInitialBehavior

// START_TEST_CASE: PduTxCyclicTransmissionAtInterval
/**
 *
 * **Purpose**
 *
 * This test verifies that cyclic transmission sends exactly when the configured cyclic
 * interval expires.
 *
 * **Given**
 *
 * * A TransmissionModeManager with cyclic transmission configured with CYCLE_INTERVAL_MS.
 *
 * **When**
 *
 * * The initial cyclic transmission at t=0 is consumed.
 * * cyclic() is called until the interval elapses.
 *
 * **Then**
 *
 * * A transmission is requested exactly when the interval expires.
 */
TEST_F(TransmissionTest, PduTxCyclicTransmissionAtInterval)
{
    // ARRANGE
    consumeInitialCyclicAtT0(transmitter_);

    // ACT AND ASSERT
    EXPECT_EQ(countCyclicSendRequests(transmitter_, CYCLE_INTERVAL_MS), 1U);
}

// END_TEST_CASE: PduTxCyclicTransmissionAtInterval

// START_TEST_CASE: PduTxCyclicTransmissionPeriodic
/**
 *
 * **Purpose**
 *
 * This test verifies that cyclic transmission repeats at each configured interval after
 * the initial transmission.
 *
 * **Given**
 *
 * * A TransmissionModeManager with cyclic transmission configured with CYCLE_INTERVAL_MS.
 *
 * **When**
 *
 * * The initial cyclic transmission at t=0 is consumed.
 * * cyclic() is called over multiple intervals.
 *
 * **Then**
 *
 * * A transmission is requested at each interval boundary.
 */
TEST_F(TransmissionTest, PduTxCyclicTransmissionPeriodic)
{
    // ARRANGE
    consumeInitialCyclicAtT0(transmitter_);

    // ACT AND ASSERT
    EXPECT_EQ(countCyclicSendRequests(transmitter_, CYCLE_INTERVAL_MS), 1U);
    EXPECT_EQ(countCyclicSendRequests(transmitter_, CYCLE_INTERVAL_MS), 1U);
    EXPECT_EQ(countCyclicSendRequests(transmitter_, CYCLE_INTERVAL_MS), 1U);
}

// END_TEST_CASE: PduTxCyclicTransmissionPeriodic

// START_TEST_CASE: PduTxCyclicTransmissionTimingAccuracy
/**
 *
 * **Purpose**
 *
 * This test verifies that cyclic transmission respects the exact interval timing and does
 * not transmit prematurely.
 *
 * **Given**
 *
 * * A TransmissionModeManager with cyclic transmission configured with CYCLE_INTERVAL_MS.
 *
 * **When**
 *
 * * The initial cyclic transmission at t=0 is consumed.
 * * cyclic() is called in sub-interval increments until the full interval is reached.
 *
 * **Then**
 *
 * * No transmission occurs until the exact interval boundary; transmission occurs only then.
 */
TEST_F(TransmissionTest, PduTxCyclicTransmissionTimingAccuracy)
{
    // ARRANGE
    consumeInitialCyclicAtT0(transmitter_);

    // ACT AND ASSERT
    for (uint8_t i = 0U; i < CYCLE_INTERVAL_MS - 1U; ++i)
    {
        EXPECT_EQ(countCyclicSendRequests(transmitter_, 1U), 0U);
    }
    EXPECT_EQ(countCyclicSendRequests(transmitter_, 1U), 1U);
}

// END_TEST_CASE: PduTxCyclicTransmissionTimingAccuracy

// START_TEST_CASE: PduTxCyclicAndEventCombined
/**
 *
 * **Purpose**
 *
 * This test verifies that cyclic and event-triggered transmissions can coexist and both
 * contribute to the transmission count independently.
 *
 * **Given**
 *
 * * A TransmissionModeManager with both cyclic and event-triggered transmission configured.
 *
 * **When**
 *
 * * The initial cyclic transmission at t=0 is consumed.
 * * An event is triggered and time advances through one cyclic interval.
 *
 * **Then**
 *
 * * Both the cyclic transmission and the event-triggered transmission are counted.
 */
TEST_F(TransmissionTest, PduTxCyclicAndEventCombined)
{
    // ARRANGE
    consumeInitialCyclicAtT0(transmitter_);
    ASSERT_FALSE(transmitter_.triggerEvent());
    ASSERT_TRUE(transmitter_.cyclic());

    // ACT AND ASSERT
    uint32_t const sends = countCyclicSendRequests(transmitter_, CYCLE_INTERVAL_MS);
    EXPECT_GE(sends, 1U);
}

// END_TEST_CASE: PduTxCyclicAndEventCombined

/**
 * \brief Test fixture for TransmissionModeSwitch tests.
 */
class TransmissionModeSwitchTest : public ::testing::Test
{
protected:
    static constexpr uint8_t DEBOUNCE_TIME_MS{5U};
    static constexpr uint8_t REPETITION_COUNT{2U};
    static constexpr uint8_t REPETITION_INTERVAL_MS{3U};
    static constexpr uint8_t CYCLE_INTERVAL_MS{10U};
    static constexpr uint8_t CYCLE_INTERVAL_MODE2_MS{5U};

    using EventTx   = EventTransmission<DEBOUNCE_TIME_MS, REPETITION_INTERVAL_MS, REPETITION_COUNT>;
    using CyclicTx  = CyclicTransmission<CYCLE_INTERVAL_MS>;
    using CyclicTx2 = CyclicTransmission<CYCLE_INTERVAL_MODE2_MS>;
    using Mode1     = TransmissionMode<EventTx, CyclicTx>;
    using Mode2     = TransmissionMode<CyclicTx2>;
    using ModeManager = TransmissionModeManager<Mode1, Mode2>;

    ModeManager transmitter_{};
};

// START_TEST_CASE: ModeSwitchRequestsCyclicTransmissionOnNextTick
/**
 *
 * **Purpose**
 *
 * This test verifies that entering a cyclic mode requests a notification on the next tick.
 *
 * **Given**
 *
 * * A TransmissionModeManager with Mode1 (cyclic 10ms) and Mode2 (cyclic 5ms) configured.
 * * Mode1 is active by default.
 * * The initial cyclic transmission at t=0 is consumed.
 *
 * **When**
 *
 * * The active condition switches to Mode2.
 * * cyclic() is called once immediately after the switch.
 *
 * **Then**
 *
 * * A mode-entry transmission is requested on that call.
 */
TEST_F(TransmissionModeSwitchTest, ModeSwitchRequestsCyclicTransmissionOnNextTick)
{
    // ARRANGE
    consumeInitialCyclicAtT0(transmitter_);
    ASSERT_TRUE(transmitter_.setActiveCondition(true));

    // ACT AND ASSERT
    EXPECT_EQ(countCyclicSendRequests(transmitter_, 1U), 1U);
}

// END_TEST_CASE: ModeSwitchRequestsCyclicTransmissionOnNextTick

// START_TEST_CASE: ModeSwitchThenFollowsNewCyclicInterval
/**
 *
 * **Purpose**
 *
 * This test verifies that after switching to Mode2, cyclic transmission follows Mode2 interval
 * from the switch point.
 *
 * **Given**
 *
 * * A TransmissionModeManager with Mode1 (cyclic 10ms) and Mode2 (cyclic 5ms) configured.
 * * Mode1 is active by default.
 *
 * **When**
 *
 * * The active condition switches to Mode2.
 * * cyclic() is called over one full Mode2 cyclic interval.
 *
 * **Then**
 *
 * * One entry notification occurs, then no further send occurs until the Mode2 boundary.
 */
TEST_F(TransmissionModeSwitchTest, ModeSwitchThenFollowsNewCyclicInterval)
{
    // ARRANGE
    ASSERT_TRUE(transmitter_.setActiveCondition(true));

    // ACT AND ASSERT
    EXPECT_EQ(countCyclicSendRequests(transmitter_, 1U), 1U);
    EXPECT_EQ(countCyclicSendRequests(transmitter_, CYCLE_INTERVAL_MODE2_MS - 2U), 0U);
    EXPECT_EQ(countCyclicSendRequests(transmitter_, 1U), 1U);
}

// END_TEST_CASE: ModeSwitchThenFollowsNewCyclicInterval

// START_TEST_CASE: ModeSwitchMaintainsContinuousPeriodicityInMode2
/**
 *
 * **Purpose**
 *
 * This test verifies continuous periodic cyclic behavior in Mode2 across two consecutive periods
 * after switching from Mode1.
 *
 * **Given**
 *
 * * A TransmissionModeManager with Mode1 (cyclic 10ms) and Mode2 (cyclic 5ms) configured.
 * * Mode1 is active by default.
 *
 * **When**
 *
 * * The active condition switches to Mode2.
 * * cyclic() is called across two full Mode2 periods.
 *
 * **Then**
 *
 * * One entry notification occurs; no further transmission occurs before each Mode2 boundary.
 * * One transmission is requested at each Mode2 boundary.
 */
TEST_F(TransmissionModeSwitchTest, ModeSwitchMaintainsContinuousPeriodicityInMode2)
{
    // ARRANGE
    ASSERT_TRUE(transmitter_.setActiveCondition(true));

    // ACT AND ASSERT - 1st Mode2 period
    EXPECT_EQ(countCyclicSendRequests(transmitter_, 1U), 1U);
    EXPECT_EQ(countCyclicSendRequests(transmitter_, CYCLE_INTERVAL_MODE2_MS - 2U), 0U);
    EXPECT_EQ(countCyclicSendRequests(transmitter_, 1U), 1U);

    // ACT AND ASSERT - 2nd Mode2 period
    EXPECT_EQ(countCyclicSendRequests(transmitter_, CYCLE_INTERVAL_MODE2_MS - 1U), 0U);
    EXPECT_EQ(countCyclicSendRequests(transmitter_, 1U), 1U);
}

// END_TEST_CASE: ModeSwitchMaintainsContinuousPeriodicityInMode2

// START_TEST_CASE: ModeSwitchBackQueuesEntryAndResetsMode1CyclicTimer
/**
 *
 * **Purpose**
 *
 * This test verifies that switching back queues an entry notification and starts Mode1 timing
 * from the switch point.
 *
 * **Given**
 *
 * * A TransmissionModeManager with Mode1 (cyclic 10ms) and Mode2 (cyclic 5ms) configured.
 * * Mode1 is active by default.
 * * The initial cyclic transmission at t=0 is consumed.
 *
 * **When**
 *
 * * The active condition switches to Mode2 and then back to Mode1.
 * * cyclic() is called immediately and then until Mode1 interval expires.
 *
 * **Then**
 *
 * * One entry notification occurs on the next tick after switching back.
 * * One transmission occurs at Mode1 interval boundary.
 */
TEST_F(TransmissionModeSwitchTest, ModeSwitchBackQueuesEntryAndResetsMode1CyclicTimer)
{
    // ARRANGE
    consumeInitialCyclicAtT0(transmitter_);
    ASSERT_TRUE(transmitter_.setActiveCondition(true));
    ASSERT_TRUE(transmitter_.setActiveCondition(false));

    // ACT AND ASSERT
    EXPECT_EQ(countCyclicSendRequests(transmitter_, 1U), 1U);
    EXPECT_EQ(countCyclicSendRequests(transmitter_, CYCLE_INTERVAL_MS - 2U), 0U);
    EXPECT_EQ(countCyclicSendRequests(transmitter_, 1U), 1U);
}

// END_TEST_CASE: ModeSwitchBackQueuesEntryAndResetsMode1CyclicTimer

// START_TEST_CASE: NoImmediateTransmissionWhenConditionUnchanged
/**
 *
 * **Purpose**
 *
 * This test verifies that calling setActiveCondition() with the same value does not trigger
 * an additional immediate cyclic transmission.
 *
 * **Given**
 *
 * * A TransmissionModeManager with Mode1 active by default.
 *
 * **When**
 *
 * * The initial cyclic transmission at t=0 is consumed.
 * * setActiveCondition(false) is called while Mode1 is already active.
 * * cyclic() is called before the cyclic interval elapses.
 *
 * **Then**
 *
 * * No immediate transmission is triggered.
 */
TEST_F(TransmissionModeSwitchTest, NoImmediateTransmissionWhenConditionUnchanged)
{
    // ARRANGE
    consumeInitialCyclicAtT0(transmitter_);

    // ACT AND ASSERT
    EXPECT_FALSE(transmitter_.setActiveCondition(false)); // already in Mode1 (default)
    EXPECT_EQ(countCyclicSendRequests(transmitter_, CYCLE_INTERVAL_MS - 1U), 0U);
}

// END_TEST_CASE: NoImmediateTransmissionWhenConditionUnchanged

// START_TEST_CASE: NoImmediateTransmissionWhenConditionUnchangedInMode2
/**
 *
 * **Purpose**
 *
 * This test verifies that calling setActiveCondition() with the same value while already in
 * Mode2 does not trigger a cyclic timer restart or additional transmission.
 *
 * **Given**
 *
 * * A TransmissionModeManager with Mode1 (cyclic 10ms) and Mode2 (cyclic 5ms) configured.
 * * Mode2 is active after switching.
 *
 * **When**
 *
 * * setActiveCondition(true) is called again while Mode2 is already active.
 * * cyclic() is called before the Mode2 interval elapses.
 *
 * **Then**
 *
 * * setActiveCondition() returns false (no mode change).
 * * No transmission is triggered before the Mode2 boundary.
 */
TEST_F(TransmissionModeSwitchTest, NoImmediateTransmissionWhenConditionUnchangedInMode2)
{
    // ARRANGE
    ASSERT_TRUE(transmitter_.setActiveCondition(true));
    ASSERT_TRUE(transmitter_.cyclic());

    // ACT AND ASSERT
    EXPECT_FALSE(transmitter_.setActiveCondition(true));
    EXPECT_EQ(countCyclicSendRequests(transmitter_, CYCLE_INTERVAL_MODE2_MS - 2U), 0U);
    EXPECT_EQ(countCyclicSendRequests(transmitter_, 1U), 1U);
}

// END_TEST_CASE: NoImmediateTransmissionWhenConditionUnchangedInMode2

/**
 * \brief Test fixture for mode switch to a mode without cyclic/event strategies.
 */
class TransmissionModeSwitchNoTransmissionTest : public ::testing::Test
{
protected:
    static constexpr uint8_t DEBOUNCE_TIME_MS{5U};
    static constexpr uint8_t REPETITION_COUNT{2U};
    static constexpr uint8_t REPETITION_INTERVAL_MS{3U};
    static constexpr uint8_t CYCLE_INTERVAL_MS{10U};

    using EventTx  = EventTransmission<DEBOUNCE_TIME_MS, REPETITION_INTERVAL_MS, REPETITION_COUNT>;
    using CyclicTx = CyclicTransmission<CYCLE_INTERVAL_MS>;
    using Mode1    = TransmissionMode<EventTx, CyclicTx>;
    using Mode2    = TransmissionMode<NoTransmission>;
    using ModeManager = TransmissionModeManager<Mode1, Mode2>;

    ModeManager transmitter_{};
};

// START_TEST_CASE: ModeSwitchToNoCyclicStrategyReturnsFalseAndCyclicStaysSilent
/**
 *
 * **Purpose**
 *
 * This test verifies that switching to a mode without cyclic strategy returns false
 * (no immediate send requested) and cyclic() remains silent.
 *
 * **Given**
 *
 * * Mode1 has cyclic/event strategies.
 * * Mode2 is NoTransmission (no cyclic strategy).
 *
 * **When**
 *
 * * The initial cyclic transmission at t=0 is consumed.
 * * setActiveCondition(true) switches to Mode2.
 * * cyclic() is called for multiple ticks.
 *
 * **Then**
 *
 * * setActiveCondition() returns false.
 * * cyclic() does not request transmission.
 */
TEST_F(
    TransmissionModeSwitchNoTransmissionTest,
    ModeSwitchToNoCyclicStrategyReturnsFalseAndCyclicStaysSilent)
{
    // ARRANGE
    consumeInitialCyclicAtT0(transmitter_);

    // ACT
    EXPECT_FALSE(transmitter_.setActiveCondition(
        true)); // no cyclic strategy, so no immediate send requested

    // ASSERT
    EXPECT_EQ(countCyclicSendRequests(transmitter_, CYCLE_INTERVAL_MS + 2U), 0U);
}

// END_TEST_CASE: ModeSwitchToNoCyclicStrategyReturnsFalseAndCyclicStaysSilent

// START_TEST_CASE: TriggerEventReturnsFalseAfterSwitchToModeWithoutEventStrategy
/**
 *
 * **Purpose**
 *
 * This test verifies that triggerEvent() returns false after switching to a mode
 * that has no EventTransmission strategy.
 *
 * **Given**
 *
 * * Mode1 has event strategy.
 * * Mode2 is NoTransmission (no trigger strategy).
 *
 * **When**
 *
 * * setActiveCondition(true) switches to Mode2.
 * * triggerEvent() is called.
 *
 * **Then**
 *
 * * triggerEvent() returns false.
 */
TEST_F(
    TransmissionModeSwitchNoTransmissionTest,
    TriggerEventReturnsFalseAfterSwitchToModeWithoutEventStrategy)
{
    // ARRANGE
    ASSERT_FALSE(transmitter_.setActiveCondition(true)); // no event strategy, so no send requested

    // ACT + ASSERT
    EXPECT_FALSE(transmitter_.triggerEvent());
}

// END_TEST_CASE: TriggerEventReturnsFalseAfterSwitchToModeWithoutEventStrategy

// START_TEST_CASE: EventTriggerDebounceCyclicUnaffected
/**
 *
 * **Purpose**
 *
 * This test verifies that debounce does not affect scheduled cyclic transmissions.
 *
 * **Given**
 *
 * * A TransmissionModeManager with both event-triggered and cyclic transmission configured.
 *
 * **When**
 *
 * * The initial cyclic transmission at t=0 is consumed.
 * * triggerEvent() is called and the cyclic interval elapses while debounce is still active.
 *
 * **Then**
 *
 * * Cyclic transmissions continue unaffected by the debounce timer.
 */
TEST(TransmissionNoRepetitionTest, EventTriggerDebounceCyclicUnaffected)
{
    // ARRANGE
    constexpr uint8_t DEBOUNCE_MS               = 20U;
    constexpr uint8_t NO_REPETITION_INTERVAL_MS = 0U;
    constexpr uint8_t NO_REPETITIONS            = 0U;
    constexpr uint8_t CYCLIC_INTERVAL_MS        = 10U;

    using EventTxNoRep = EventTransmission<DEBOUNCE_MS, NO_REPETITION_INTERVAL_MS, NO_REPETITIONS>;
    using Mode         = TransmissionMode<EventTxNoRep, CyclicTransmission<CYCLIC_INTERVAL_MS>>;
    TransmissionModeManager<Mode> transmitter{};

    consumeInitialCyclicAtT0(transmitter);

    ASSERT_FALSE(transmitter.triggerEvent());
    ASSERT_TRUE(transmitter.cyclic());

    // ACT + ASSERT
    // The event send advances the cyclic timer, so the boundary is one tick closer.
    EXPECT_EQ(countCyclicSendRequests(transmitter, CYCLIC_INTERVAL_MS - 2U), 0U);

    // Cyclic send at its boundary, even while debounce remains active (20ms > 10ms).
    EXPECT_EQ(countCyclicSendRequests(transmitter, 1U), 1U);
}

// END_TEST_CASE: EventTriggerDebounceCyclicUnaffected

} // namespace middleware::core::test
