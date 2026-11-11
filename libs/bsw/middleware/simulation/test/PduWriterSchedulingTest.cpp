/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/Cluster0Cluster.h"
#include "middleware/shm/Config.h"
#include "middleware/shm/QueueDefinitions.h"
#include "org/test/signal/samplePduWithCrcWriter.h"
#include "org/test/signal/samplePduWithDatatypesReader.h"
#include "org/test/signal/samplePduWithDatatypesWriter.h"
#include "time/mock/SystemTimerProviderMock.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <etl/limits.h>
#include <cstdint>
#include <new>
#include <type_traits>

namespace middleware::shm
{
MemoryLayout& getMemoryLayout()
{
    static MemoryLayout layout;
    return layout;
}
} // namespace middleware::shm

namespace
{
using namespace org::test::signal;

constexpr uint32_t CYCLIC_TICKS{5000U};
constexpr uint32_t REPETITION_TICKS{1000U};
constexpr uint32_t DEBOUNCE_TICKS{100U};

static_assert(!std::is_copy_constructible<samplePduWithCrcWriter>::value);
static_assert(!std::is_move_constructible<samplePduWithCrcWriter>::value);

class PduWriterSchedulingTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto& layout = middleware::shm::getMemoryLayout();
        layout.~MemoryLayout();
        new (&layout) middleware::shm::MemoryLayout();
        middleware::time::test::setSystemTimerProviderMock(&timerMock);
        queue_ = middleware::shm::getOutboundPduQueue();
    }

    void TearDown() override { middleware::time::test::unsetSystemTimerProviderMock(); }

    /// Removes all pending notifications between independent timing checks.
    void drain()
    {
        middleware::shm::OutboundPduQueue::Receiver receiver{*queue_};
        while (!receiver.isEmpty())
        {
            receiver.advance();
        }
    }

    /// Runs the cluster tick that services all registered writers once per simulated millisecond.
    void advanceTicks(uint32_t const ticks)
    {
        for (uint32_t tick = 0U; tick < ticks; ++tick)
        {
            middleware::updateCluster0ClusterTimeouts();
        }
    }

    /// Consumes the initially due cyclic send and the first period that follows it.
    void skipInitialAndFirstCyclicSend()
    {
        advanceTicks(1U);
        drain();
        advanceTicks(CYCLIC_TICKS);
        drain();
    }

    static samplePduWithCrc zeroCrcPdu() { return samplePduWithCrc{0U, 0U, 0U, 0U, 0U}; }

    static samplePduWithDatatypes zeroDatatypesPdu()
    {
        return samplePduWithDatatypes{
            0U, 0U, {0U, 0U}, ExampleEnum::Value0, ExampleBitset{0U}, false};
    }

    ::testing::NiceMock<middleware::time::test::SystemTimerProviderMock> timerMock;
    middleware::shm::OutboundPduQueue* queue_{};
};

TEST_F(PduWriterSchedulingTest, ConstructionStagesAndCyclicPublicationFollowsPeriod)
{
    samplePduWithCrcWriter writer{zeroCrcPdu()};
    EXPECT_TRUE(queue_->isEmpty()) << "Construction must only stage the initial values";

    advanceTicks(1U);
    ASSERT_EQ(queue_->size(), 1U) << "The first tick must publish the initially due cyclic PDU";
    middleware::shm::OutboundPduQueue::Receiver receiver{*queue_};
    EXPECT_EQ(receiver.peek(), middleware::shm::PDU_ID_SAMPLEPDUWITHCRC);
    drain();

    advanceTicks(CYCLIC_TICKS - 1U);
    EXPECT_TRUE(queue_->isEmpty()) << "Cyclic publication must not run before its period";
    advanceTicks(1U);
    EXPECT_EQ(queue_->size(), 1U) << "Cyclic publication must run when its period elapses";
}

TEST_F(PduWriterSchedulingTest, ModeChangesAndEventsArePublishedOnClusterTicks)
{
    samplePduWithCrcWriter writer{zeroCrcPdu()};
    skipInitialAndFirstCyclicSend();

    writer.setActiveCondition(true);
    advanceTicks(REPETITION_TICKS);
    writer.triggerPduEvent();
    EXPECT_TRUE(queue_->isEmpty()) << "The empty mode must suppress cyclic and event sends";
    writer.setActiveCondition(false);
    EXPECT_TRUE(queue_->isEmpty())
        << "A mode change must defer notification until the cluster tick";
    advanceTicks(1U);
    EXPECT_EQ(queue_->size(), 1U) << "The cluster tick must publish after entering a cyclic mode";
    drain();

    writer.triggerPduEvent();
    writer.triggerPduEvent();
    EXPECT_TRUE(queue_->isEmpty()) << "Accepted events must wait for the cluster tick";
    advanceTicks(1U);
    EXPECT_EQ(queue_->size(), 1U) << "The cluster tick must enqueue an accepted event";
    drain();

    writer.triggerPduEvent();
    EXPECT_TRUE(queue_->isEmpty()) << "An event inside the debounce window must remain pending";
    advanceTicks(DEBOUNCE_TICKS - 1U);
    EXPECT_TRUE(queue_->isEmpty()) << "A pending event must not send before debounce expires";
    advanceTicks(1U);
    EXPECT_EQ(queue_->size(), 1U) << "The pending event must send when debounce expires";
    drain();
    advanceTicks(REPETITION_TICKS);
    EXPECT_EQ(queue_->size(), 1U) << "The writer must publish the first event repetition";
    drain();
    advanceTicks(REPETITION_TICKS);
    EXPECT_EQ(queue_->size(), 1U) << "The writer must publish the second event repetition";
    drain();
    advanceTicks(REPETITION_TICKS);
    EXPECT_TRUE(queue_->isEmpty()) << "No additional repetition may be generated";
}

TEST_F(PduWriterSchedulingTest, FullQueueDropsNotificationButKeepsCommittedPayload)
{
    samplePduWithCrcWriter writer{zeroCrcPdu()};
    advanceTicks(1U);
    drain();

    middleware::shm::OutboundPduQueue::Sender sender{*queue_};
    while (!queue_->isFull())
    {
        ASSERT_TRUE(sender.write(middleware::shm::PDU_ID_SAMPLEPDUWITHCRC));
    }
    auto const lostBefore = queue_->getStats().lostMessages;

    writer.writeSampleSignal(9U);
    writer.commit();
    EXPECT_TRUE(queue_->isFull()) << "Committing a payload must not depend on queue capacity";
    EXPECT_EQ(queue_->getStats().lostMessages, lostBefore)
        << "Committing a payload must not attempt a queue notification";

    advanceTicks(CYCLIC_TICKS);
    EXPECT_EQ(queue_->getStats().lostMessages, lostBefore + 1U)
        << "A failed scheduled notification must be counted exactly once";
    samplePduWithCrcBuffer payload{};
    middleware::shm::getMemoryLayout().samplePduWithCrc.load(payload);
    EXPECT_EQ(payload[1], 0x90U) << "Queue failure must not roll back the shared payload";

    drain();
    writer.commit();
    EXPECT_TRUE(queue_->isEmpty()) << "An explicit commit must not announce the PDU";
}

TEST_F(PduWriterSchedulingTest, AllStrategiesExecuteWhenCyclicAndEventAreDueTogether)
{
    samplePduWithCrcWriter writer{zeroCrcPdu()};
    writer.commit();
    advanceTicks(1U);
    EXPECT_EQ(queue_->size(), 1U) << "A new writer must publish its initially due cyclic PDU";
    drain();

    advanceTicks(CYCLIC_TICKS - DEBOUNCE_TICKS - 1U);
    writer.triggerPduEvent();
    EXPECT_TRUE(queue_->isEmpty()) << "An event request must wait for the cluster tick";
    advanceTicks(1U);
    EXPECT_EQ(queue_->size(), 1U) << "The pending event must be sent on the cluster tick";
    drain();

    advanceTicks(DEBOUNCE_TICKS - 1U);
    EXPECT_TRUE(queue_->isEmpty()) << "The second event must remain pending during debounce";
    writer.triggerPduEvent();
    advanceTicks(1U);
    EXPECT_EQ(queue_->size(), 1U)
        << "The cyclic and expired event requests must be combined on the same tick";
    drain();

    advanceTicks(REPETITION_TICKS);
    ASSERT_EQ(queue_->size(), 1U) << "The first event repetition must be retained";
    drain();
    advanceTicks(REPETITION_TICKS);
    ASSERT_EQ(queue_->size(), 1U) << "The second event repetition must be retained";
    drain();

    advanceTicks(CYCLIC_TICKS - (2U * REPETITION_TICKS) - 1U);
    EXPECT_TRUE(queue_->isEmpty()) << "The cyclic deadline must restart on the combined send";
    advanceTicks(1U);
    EXPECT_EQ(queue_->size(), 1U) << "The cyclic strategy must retain its configured period";
}

TEST_F(PduWriterSchedulingTest, AggregateWriteQueuesOnTriggerOnChangeSignals)
{
    samplePduWithDatatypesWriter writer{zeroDatatypesPdu()};
    writer.write(
        samplePduWithDatatypes{1U, 0U, {0U, 0U}, ExampleEnum::Value0, ExampleBitset{0U}, false});
    EXPECT_TRUE(queue_->isEmpty()) << "Aggregate writes must not enqueue before a cluster tick";
    writer.commit();
    advanceTicks(1U);
    EXPECT_EQ(queue_->size(), 1U) << "Aggregate trigger_on_change must queue on cyclic()";
}

TEST_F(PduWriterSchedulingTest, SignalChangesAreCoalescedAndPublishedAfterDebounce)
{
    samplePduWithDatatypesWriter writer{zeroDatatypesPdu()};

    writer.writeFirstSignal(2U);
    EXPECT_TRUE(queue_->isEmpty()) << "A new wire value must defer its notification";
    writer.commit();
    advanceTicks(1U);
    EXPECT_EQ(queue_->size(), 1U) << "A new wire value must queue on cyclic()";
    drain();

    writer.writeFirstSignal(3U);
    EXPECT_TRUE(queue_->isEmpty()) << "Signal-triggered events must honor debounce";
    writer.commit();
    advanceTicks(1U);
    EXPECT_TRUE(queue_->isEmpty()) << "A debounced signal change must not queue";
    writer.writeFirstSignal(4U);
    writer.commit();
    advanceTicks(DEBOUNCE_TICKS - 2U);
    EXPECT_TRUE(queue_->isEmpty()) << "Coalesced changes must wait for debounce expiry";
    advanceTicks(1U);
    ASSERT_EQ(queue_->size(), 1U) << "Debounce expiry must publish the latest change once";
    middleware::shm::OutboundPduQueue::Receiver receiver{*queue_};
    EXPECT_EQ(receiver.peek(), middleware::shm::PDU_ID_SAMPLEPDUWITHDATATYPES);
    samplePduWithDatatypesReader reader{nullptr};
    EXPECT_EQ(reader.read().firstSignal, 4U);
    drain();
    writer.writeFirstSignal(4U);
    writer.commit();
    advanceTicks(REPETITION_TICKS);
    EXPECT_TRUE(queue_->isEmpty()) << "An unchanged value must not create another event";
}

TEST_F(PduWriterSchedulingTest, BitsetChangesQueueAndRoundTrip)
{
    samplePduWithDatatypesWriter writer{zeroDatatypesPdu()};

    writer.writeBitsetSignal(ExampleBitset{0x15U});
    EXPECT_TRUE(queue_->isEmpty()) << "A bitset change must defer its notification";
    writer.commit();
    advanceTicks(1U);
    EXPECT_EQ(queue_->size(), 1U) << "A changed bitset must queue on cyclic()";
    drain();

    samplePduWithDatatypesReader reader{nullptr};
    auto const decoded = reader.read();
    EXPECT_TRUE(decoded.bitsetSignal.testEnabled()) << "Bit 0 must round trip";
    EXPECT_FALSE(decoded.bitsetSignal.testFault()) << "Unset bit 1 must remain clear";
    EXPECT_TRUE(decoded.bitsetSignal.testActive()) << "Bit 2 must round trip";
    EXPECT_FALSE(decoded.bitsetSignal.testLocked()) << "Unset bit 3 must remain clear";
    EXPECT_TRUE(decoded.bitsetSignal.testOverheated()) << "Bit 4 must round trip";
    EXPECT_FALSE(decoded.boolSignal) << "Bitset write must not corrupt the following signal";

    advanceTicks(DEBOUNCE_TICKS);
    writer.writeBitsetSignal(ExampleBitset{0x15U});
    writer.commit();
    advanceTicks(1U);
    EXPECT_TRUE(queue_->isEmpty()) << "An unchanged bitset must not trigger";
}

TEST_F(PduWriterSchedulingTest, EnumChangesQueueOnlyWhenTheValueDiffers)
{
    samplePduWithDatatypesWriter writer{zeroDatatypesPdu()};

    writer.writeEnumSignal(ExampleEnum::Value1);
    EXPECT_TRUE(queue_->isEmpty()) << "An enum change must defer its notification";
    writer.commit();
    advanceTicks(1U);
    EXPECT_EQ(queue_->size(), 1U) << "A changed enum must queue on cyclic()";
    drain();

    advanceTicks(DEBOUNCE_TICKS);
    writer.writeEnumSignal(ExampleEnum::Value1);
    advanceTicks(1U);
    EXPECT_TRUE(queue_->isEmpty()) << "An unchanged enum must not trigger";
}

TEST_F(PduWriterSchedulingTest, PhysicalWriteConvertsToRawAndQueues)
{
    samplePduWithDatatypesWriter writer{zeroDatatypesPdu()};

    writer.writeFirstSignalFromPhysical(2.0);
    EXPECT_TRUE(queue_->isEmpty()) << "A physical-value write must defer its notification";
    writer.commit();
    advanceTicks(1U);
    EXPECT_EQ(queue_->size(), 1U) << "A physical-value write must queue on cyclic()";

    samplePduWithDatatypesReader reader{nullptr};
    EXPECT_EQ(reader.read().firstSignal, 4U)
        << "Physical conversion must produce the expected raw signal";
}

TEST_F(PduWriterSchedulingTest, PhysicalWriteSaturatesAndPreservesFractionalTruncation)
{
    struct PhysicalCase
    {
        double value;
        uint8_t expected;
    };

    constexpr size_t CASE_COUNT{8U};
    constexpr uint8_t MAXIMUM_RAW{0x1FU};
    etl::array<PhysicalCase, CASE_COUNT> const cases{{
        {-1.0, 0U},
        {-etl::numeric_limits<double>::infinity(), 0U},
        {0.0, 0U},
        {0.75, 1U},
        {15.5, MAXIMUM_RAW},
        {16.0, MAXIMUM_RAW},
        {etl::numeric_limits<double>::infinity(), MAXIMUM_RAW},
        {etl::numeric_limits<double>::max(), MAXIMUM_RAW},
    }};
    samplePduWithDatatypesWriter writer{zeroDatatypesPdu()};
    samplePduWithDatatypesReader reader{nullptr};
    for (auto const& sample : cases)
    {
        writer.writeFirstSignalFromPhysical(sample.value);
        writer.commit();
        EXPECT_EQ(reader.read().firstSignal, sample.expected);
    }
}

TEST_F(PduWriterSchedulingTest, PhysicalNaNLeavesPayloadUnchangedAndDoesNotTrigger)
{
    constexpr uint8_t INITIAL_RAW{4U};
    samplePduWithDatatypesWriter writer{zeroDatatypesPdu()};
    writer.writeFirstSignal(INITIAL_RAW);
    writer.commit();
    advanceTicks(1U);
    drain();

    writer.writeFirstSignalFromPhysical(etl::numeric_limits<double>::quiet_NaN());
    writer.commit();
    advanceTicks(DEBOUNCE_TICKS);
    EXPECT_TRUE(queue_->isEmpty());
    samplePduWithDatatypesReader reader{nullptr};
    EXPECT_EQ(reader.read().firstSignal, INITIAL_RAW);
}

TEST_F(PduWriterSchedulingTest, DefaultNarrowIntegralDoesNotTriggerForAllOnesWireValue)
{
    constexpr uint8_t ALL_ONES{0x1FU};
    samplePduWithDatatypesWriter writer{};
    writer.writeFirstSignal(ALL_ONES);
    writer.commit();
    advanceTicks(1U);
    EXPECT_TRUE(queue_->isEmpty());

    samplePduWithDatatypesReader reader{nullptr};
    EXPECT_EQ(reader.read().firstSignal, ALL_ONES);
}

TEST_F(PduWriterSchedulingTest, DefaultNarrowEnumDoesNotTriggerForAllOnesWireValue)
{
    samplePduWithDatatypesWriter writer{};
    writer.writeEnumSignal(ExampleEnum::ErrorValue);
    writer.commit();
    advanceTicks(1U);
    EXPECT_TRUE(queue_->isEmpty());

    samplePduWithDatatypesReader reader{nullptr};
    EXPECT_EQ(reader.read().enumSignal, ExampleEnum::ErrorValue);
}

TEST_F(PduWriterSchedulingTest, DefaultAggregateWriteDoesNotTriggerForUnchangedWireValues)
{
    samplePduWithDatatypesWriter writer{};
    writer.commit();
    samplePduWithDatatypesReader reader{nullptr};
    writer.write(reader.read());
    writer.commit();
    advanceTicks(1U);
    EXPECT_TRUE(queue_->isEmpty());
}

TEST_F(PduWriterSchedulingTest, DefaultCrcWriterEncodesAllBitsSet)
{
    samplePduWithCrcWriter writer{};
    writer.commit();
    samplePduWithCrcBuffer payload{};
    middleware::shm::getMemoryLayout().samplePduWithCrc.load(payload);
    for (auto const byte : payload)
    {
        EXPECT_EQ(byte, 0xFFU) << "Default staged signals must encode as all bits set";
    }
}

TEST_F(PduWriterSchedulingTest, DefaultDatatypesWriterEncodesAllBitsSet)
{
    samplePduWithDatatypesWriter writer{};
    writer.commit();
    samplePduWithDatatypesReader reader{nullptr};
    auto const decoded = reader.read();
    EXPECT_EQ(decoded.firstSignal, 0x1FU) << "Integral signals must have all bits set";
    EXPECT_EQ(decoded.secondSignal, 0x0FFFFFFFFFFFFFFFULL) << "Wide signals must be all ones";
    EXPECT_EQ(decoded.arraySignal[0], 0xFFU) << "Array elements must be all ones";
    EXPECT_EQ(decoded.arraySignal[1], 0xFFU) << "Array elements must be all ones";
    EXPECT_EQ(decoded.enumSignal, ExampleEnum::ErrorValue) << "Enum bits must all be set";
    EXPECT_TRUE(decoded.bitsetSignal.testOverheated()) << "Bitset bits must all be set";
    EXPECT_TRUE(decoded.boolSignal) << "Bool signals must be set";
}
} // namespace
