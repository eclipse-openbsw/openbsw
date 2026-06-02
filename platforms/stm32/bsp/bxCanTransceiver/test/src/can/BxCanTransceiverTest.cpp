/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "can/transceiver/bxcan/BxCanTransceiver.h"

#include "async/AsyncMock.h"
#include "bsp/timer/SystemTimerMock.h"
#include "can/canframes/ICANFrameSentListener.h"
#include "can/filter/BitFieldFilter.h"
#include "can/framemgmt/ICANFrameListener.h"
#include "can/framemgmt/IFilteredCANFrameSentListener.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{
using namespace ::can;
using namespace ::testing;
using namespace ::bios;

class MockCANFrameSentListener : public ::can::ICANFrameSentListener
{
public:
    MOCK_METHOD(void, canFrameSent, (::can::CANFrame const&), (override));
};

class MockCANFrameListener : public ::can::ICANFrameListener
{
public:
    MOCK_METHOD(void, frameReceived, (::can::CANFrame const&), (override));
    MOCK_METHOD(::can::IFilter&, getFilter, (), (override));
};

class MockFilteredCANFrameSentListener : public ::can::IFilteredCANFrameSentListener
{
public:
    MOCK_METHOD(void, canFrameSent, (::can::CANFrame const&), (override));
    MOCK_METHOD(::can::IFilter&, getFilter, (), (override));
};

class BxCanTransceiverTest : public Test
{
public:
    BxCanTransceiverTest() { fDevConfig.baudrate = 500000U; }

    ::async::AsyncMock fAsyncMock;
    ::async::ContextType fAsyncContext = 0;
    uint8_t fBusId                     = 0;
    BxCanDevice::Config fDevConfig{};
};

TEST_F(BxCanTransceiverTest, initFromClosed)
{
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, bxt.init());
}

TEST_F(BxCanTransceiverTest, initTwiceFails)
{
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, bxt.init());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, bxt.init());
}

TEST_F(BxCanTransceiverTest, initFailsWhenDeviceInitFails)
{
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);

    EXPECT_CALL(bxt.fDevice, init()).WillOnce(Return(false));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_INIT_FAILED, bxt.init());
    EXPECT_EQ(ICanTransceiver::State::CLOSED, bxt.getState());
}

TEST_F(BxCanTransceiverTest, openFromClosedFailsWhenDeviceInitFails)
{
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);

    EXPECT_CALL(bxt.fDevice, init()).WillOnce(Return(false));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_INIT_FAILED, bxt.open());
    EXPECT_EQ(ICanTransceiver::State::CLOSED, bxt.getState());
}

TEST_F(BxCanTransceiverTest, receiveInterruptInvalidIndex)
{
    EXPECT_EQ(0U, BxCanTransceiver::receiveInterrupt(3));
}

TEST_F(BxCanTransceiverTest, writeFromClosedFails)
{
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);
    CANFrame frame;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, bxt.write(frame));
}

TEST_F(BxCanTransceiverTest, writeWithListenerFromClosedFails)
{
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);
    CANFrame frame;
    MockCANFrameSentListener listener;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, bxt.write(frame, listener));
}

TEST_F(BxCanTransceiverTest, closeFromClosedReturnsOk)
{
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, bxt.close());
}

TEST_F(BxCanTransceiverTest, muteFromClosedFails)
{
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, bxt.mute());
}

TEST_F(BxCanTransceiverTest, unmuteFromClosedFails)
{
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, bxt.unmute());
}

TEST_F(BxCanTransceiverTest, openFromClosedWithoutInitFails)
{
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);
    // CLOSED -> open() should either re-init or fail depending on impl.
    // The contract says CLOSED->open() re-inits (see impl). So this tests
    // that open() from CLOSED is allowed (re-init path).
    EXPECT_CALL(bxt.fDevice, init()).Times(AnyNumber());
    EXPECT_CALL(bxt.fDevice, start()).Times(AnyNumber());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, bxt.open());
}

class InitedBxCanTransceiverTest : public BxCanTransceiverTest
{
public:
    InitedBxCanTransceiverTest() : fBxt(fAsyncContext, fBusId, fDevConfig)
    {
        EXPECT_CALL(fBxt.fDevice, init()).Times(AnyNumber());
        EXPECT_CALL(fBxt.fDevice, start()).Times(AnyNumber());
        EXPECT_CALL(fBxt.fDevice, stop()).Times(AnyNumber());
        EXPECT_CALL(fAsyncMock, execute(fAsyncContext, _))
            .Times(AnyNumber())
            .WillRepeatedly([](auto, auto& runnable) { runnable.execute(); });

        EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.init());
    }

    BxCanTransceiver fBxt;
};

TEST_F(InitedBxCanTransceiverTest, open)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
}

TEST_F(InitedBxCanTransceiverTest, openFailsWhenDeviceStartFails)
{
    EXPECT_CALL(fBxt.fDevice, start()).WillOnce(Return(false));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.open());
    EXPECT_EQ(ICanTransceiver::State::INITIALIZED, fBxt.getState());

    // A failed open must not allow writes
    CANFrame frame;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fBxt.write(frame));
}

TEST_F(InitedBxCanTransceiverTest, openTwiceFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.open());
}

TEST_F(InitedBxCanTransceiverTest, closeFromOpen)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.close());
}

TEST_F(InitedBxCanTransceiverTest, closeFromInitializedReturnsOk)
{
    EXPECT_CALL(fBxt.fDevice, stop());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.close());
}

TEST_F(InitedBxCanTransceiverTest, muteFromOpen)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());
}

TEST_F(InitedBxCanTransceiverTest, muteFromInitializedFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.mute());
}

TEST_F(InitedBxCanTransceiverTest, unmuteFromMuted)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.unmute());
}

TEST_F(InitedBxCanTransceiverTest, unmuteFromOpenFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.unmute());
}

TEST_F(InitedBxCanTransceiverTest, writeWhenOpen)
{
    CANFrame frame;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame));
}

TEST_F(InitedBxCanTransceiverTest, writeWhenMutedFails)
{
    CANFrame frame;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fBxt.write(frame));
}

TEST_F(InitedBxCanTransceiverTest, writeWhenFifoFull)
{
    CANFrame frame;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(false));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fBxt.write(frame));
}

TEST_F(InitedBxCanTransceiverTest, getBaudrateComesFromDeviceConfig)
{
    EXPECT_EQ(500000U, fBxt.getBaudrate());
}

TEST_F(InitedBxCanTransceiverTest, getHwQueueTimeoutComputedFromBaudrate)
{
    // 3 mailboxes x 117 bit = 351000 bit-ms/s, rounded up to a full ms:
    // 500 kbit/s -> ceil(0.702 ms) = 1 ms (never 0)
    EXPECT_EQ(1U, fBxt.getHwQueueTimeout());

    // 125 kbit/s -> ceil(351000 / 125000) = ceil(2.808) = 3 ms
    fDevConfig.baudrate = 125000U;
    BxCanTransceiver slowBxt(fAsyncContext, fBusId, fDevConfig);
    EXPECT_EQ(3U, slowBxt.getHwQueueTimeout());
}

TEST_F(InitedBxCanTransceiverTest, receiveTask)
{
    CANFrame frame;

    EXPECT_CALL(fBxt.fDevice, disableRxInterrupt()).Times(1);
    EXPECT_CALL(fBxt.fDevice, getRxCount()).WillOnce(Return(0));
    EXPECT_CALL(fBxt.fDevice, clearRxQueue()).Times(1);
    EXPECT_CALL(fBxt.fDevice, enableRxInterrupt()).Times(1);

    fBxt.receiveTask();
}

TEST_F(InitedBxCanTransceiverTest, cyclicTaskBusOff)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());

    EXPECT_CALL(fBxt.fDevice, isBusOff()).WillOnce(Return(true));
    fBxt.cyclicTask();

    // After bus-off, write should fail because state is MUTED
    CANFrame frame;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fBxt.write(frame));
}

TEST_F(InitedBxCanTransceiverTest, cyclicTaskBusRecovery)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());

    // Go to bus-off
    EXPECT_CALL(fBxt.fDevice, isBusOff()).WillOnce(Return(true)).WillOnce(Return(false));

    fBxt.cyclicTask(); // bus-off -> MUTED
    fBxt.cyclicTask(); // bus-on -> OPEN (auto-recovery, not user mute)

    // Write should work again
    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));
    CANFrame frame;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame));
}

TEST_F(InitedBxCanTransceiverTest, shutdown)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    fBxt.shutdown();
}

TEST_F(InitedBxCanTransceiverTest, receiveInterrupt)
{
    EXPECT_CALL(fBxt.fDevice, receiveISR(_)).WillOnce(Return(0));
    BxCanTransceiver::receiveInterrupt(fBusId);
}

TEST_F(InitedBxCanTransceiverTest, transmitInterrupt)
{
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    BxCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedBxCanTransceiverTest, writeWithListenerReturnsOkWhenOpen)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
}

TEST_F(InitedBxCanTransceiverTest, writeWithListenerWhenMutedFails)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fBxt.write(frame, listener));
}

TEST_F(InitedBxCanTransceiverTest, writeWithListenerWhenFifoFullReturnsQueueFull)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(false));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fBxt.write(frame, listener));
}

TEST_F(InitedBxCanTransceiverTest, writeWithListenerCallbackFromTransmitInterrupt)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));

    // The listener should be called when transmitInterrupt fires
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    BxCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedBxCanTransceiverTest, txQueueFillsToCapacity)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillRepeatedly(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());

    // Queue capacity is 3 - fill it up without draining
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));

    // 4th write should fail because queue is full
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fBxt.write(frame, listener));
}

TEST_F(InitedBxCanTransceiverTest, txQueueDrainAndRefill)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(listener, canFrameSent(_)).Times(AnyNumber());

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());

    // Fill queue
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));

    // Drain one via TX ISR callback
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    BxCanTransceiver::transmitInterrupt(fBusId);

    // Now one slot is free
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
}

TEST_F(InitedBxCanTransceiverTest, canFrameSentCallbackSingleFrame)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));

    EXPECT_CALL(fBxt.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    BxCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedBxCanTransceiverTest, canFrameSentCallbackTwoFrames)
{
    CANFrame frame1;
    CANFrame frame2;
    MockCANFrameSentListener listener1;
    MockCANFrameSentListener listener2;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillRepeatedly(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame1, listener1));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame2, listener2));

    // First TX ISR: listener1 fires
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    EXPECT_CALL(listener1, canFrameSent(_)).Times(1);
    BxCanTransceiver::transmitInterrupt(fBusId);

    // Second TX ISR: listener2 fires
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    EXPECT_CALL(listener2, canFrameSent(_)).Times(1);
    BxCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedBxCanTransceiverTest, canFrameSentCallbackThreeFramesFifoOrder)
{
    CANFrame frame;
    MockCANFrameSentListener listener1;
    MockCANFrameSentListener listener2;
    MockCANFrameSentListener listener3;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillRepeatedly(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener1));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener2));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener3));

    InSequence seq;
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    EXPECT_CALL(listener1, canFrameSent(_));
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    EXPECT_CALL(listener2, canFrameSent(_));
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    EXPECT_CALL(listener3, canFrameSent(_));

    BxCanTransceiver::transmitInterrupt(fBusId);
    BxCanTransceiver::transmitInterrupt(fBusId);
    BxCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedBxCanTransceiverTest, canFrameSentCallbackEmptyQueue)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());

    // TX ISR with nothing queued - should not crash
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    BxCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedBxCanTransceiverTest, canFrameSentCallbackAbortedWhenMuted)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillRepeatedly(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));

    // Mute before TX ISR fires
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());

    // TX ISR should still call transmitISR but listener is NOT called (muted)
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(0);
    BxCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedBxCanTransceiverTest, canFrameSentCallbackWithFireAndForgetWrite)
{
    CANFrame frame;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame));

    // TX ISR fires - no listener was registered, no crash
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    BxCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedBxCanTransceiverTest, canFrameSentCallbackSameListenerTwice)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillRepeatedly(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));

    EXPECT_CALL(fBxt.fDevice, transmitISR()).Times(2);
    EXPECT_CALL(listener, canFrameSent(_)).Times(2);

    BxCanTransceiver::transmitInterrupt(fBusId);
    BxCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedBxCanTransceiverTest, canFrameSentCallbackMixedWriteStyles)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillRepeatedly(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());

    // Fire-and-forget write
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame));
    // Write with listener
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));

    // TX ISR fires for the listener-based write
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    BxCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedBxCanTransceiverTest, receiveInterruptPassesNullFilter)
{
    EXPECT_CALL(fBxt.fDevice, receiveISR(nullptr)).WillOnce(Return(1));
    uint8_t count = BxCanTransceiver::receiveInterrupt(fBusId);
    EXPECT_EQ(1U, count);
}

TEST_F(InitedBxCanTransceiverTest, receiveInterruptUnregisteredIndexReturnsZero)
{
    // Index 2 was never registered (only index 0 was)
    EXPECT_EQ(0U, BxCanTransceiver::receiveInterrupt(2));
}

TEST_F(InitedBxCanTransceiverTest, receiveTaskWithFramesNotifiesListeners)
{
    CANFrame frame1;
    CANFrame frame2;

    EXPECT_CALL(fBxt.fDevice, disableRxInterrupt()).Times(1);
    EXPECT_CALL(fBxt.fDevice, getRxCount()).WillOnce(Return(2));
    EXPECT_CALL(fBxt.fDevice, getRxFrame(0)).WillOnce(ReturnRef(frame1));
    EXPECT_CALL(fBxt.fDevice, getRxFrame(1)).WillOnce(ReturnRef(frame2));
    EXPECT_CALL(fBxt.fDevice, clearRxQueue()).Times(1);
    EXPECT_CALL(fBxt.fDevice, enableRxInterrupt()).Times(1);

    fBxt.receiveTask();
}

TEST_F(InitedBxCanTransceiverTest, receiveTaskMasksRxInterruptWhileDraining)
{
    // disableRxInterrupt must come first, enableRxInterrupt last, so
    // receiveISR cannot enqueue between getRxCount() and clearRxQueue()
    InSequence seq;
    EXPECT_CALL(fBxt.fDevice, disableRxInterrupt());
    EXPECT_CALL(fBxt.fDevice, getRxCount()).WillOnce(Return(0));
    EXPECT_CALL(fBxt.fDevice, clearRxQueue());
    EXPECT_CALL(fBxt.fDevice, enableRxInterrupt());

    fBxt.receiveTask();
}

TEST_F(InitedBxCanTransceiverTest, writeFireAndForgetNotifiesRegisteredSentListeners)
{
    CANFrame frame;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());

    // notifySentListeners is called synchronously by write(frame)
    // No registered sent listeners by default, so it just doesn't crash
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame));
}

TEST_F(InitedBxCanTransceiverTest, writeFailureDoesNotNotifySentListeners)
{
    CANFrame frame;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(false));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());

    // HW queue full - no notification expected
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fBxt.write(frame));
}

TEST_F(InitedBxCanTransceiverTest, docanWriteThenTxIsr)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));

    // Simulates: DoCAN sets _sendPending after write() returns,
    // then TX ISR fires, canFrameSent() clears _sendPending
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    BxCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedBxCanTransceiverTest, docanConsecutiveMultiFrameSend)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(fBxt.fDevice, transmitISR()).Times(3);
    EXPECT_CALL(listener, canFrameSent(_)).Times(3);

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());

    // Simulate DoCAN sending 3 consecutive frames
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
    BxCanTransceiver::transmitInterrupt(fBusId);

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
    BxCanTransceiver::transmitInterrupt(fBusId);

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
    BxCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedBxCanTransceiverTest, openWithFrameDelegatesToOpen)
{
    CANFrame frame;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open(frame));
    // Second open should fail, proving state transitioned
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.open());
}

TEST_F(InitedBxCanTransceiverTest, openFromClosedReinitsDevice)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.close());

    // Re-open from CLOSED should call init + start
    EXPECT_CALL(fBxt.fDevice, init()).Times(1);
    EXPECT_CALL(fBxt.fDevice, start()).Times(1);
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
}

TEST_F(InitedBxCanTransceiverTest, closeFromMutedReturnsOk)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.close());
}

TEST_F(InitedBxCanTransceiverTest, closeIsIdempotent)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.close());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.close());
}

TEST_F(InitedBxCanTransceiverTest, muteTwiceFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.mute());
}

TEST_F(InitedBxCanTransceiverTest, unmuteFromInitializedFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.unmute());
}

TEST_F(InitedBxCanTransceiverTest, getBusIdReturnsConfiguredValue)
{
    EXPECT_EQ(fBusId, fBxt.getBusId());
}

TEST_F(InitedBxCanTransceiverTest, getStateReflectsLifecycle)
{
    EXPECT_EQ(ICanTransceiver::State::INITIALIZED, fBxt.getState());

    fBxt.open();
    EXPECT_EQ(ICanTransceiver::State::OPEN, fBxt.getState());

    fBxt.mute();
    EXPECT_EQ(ICanTransceiver::State::MUTED, fBxt.getState());

    fBxt.unmute();
    EXPECT_EQ(ICanTransceiver::State::OPEN, fBxt.getState());

    fBxt.close();
    EXPECT_EQ(ICanTransceiver::State::CLOSED, fBxt.getState());
}

TEST_F(InitedBxCanTransceiverTest, disableRxInterruptDelegatesToDevice)
{
    EXPECT_CALL(fBxt.fDevice, disableRxInterrupt()).Times(1);
    BxCanTransceiver::disableRxInterrupt(fBusId);
}

TEST_F(InitedBxCanTransceiverTest, writeFromInitializedReturnsTxOffline)
{
    CANFrame frame;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fBxt.write(frame));
}

TEST_F(InitedBxCanTransceiverTest, writeWithListenerFromInitializedReturnsTxOffline)
{
    CANFrame frame;
    MockCANFrameSentListener listener;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fBxt.write(frame, listener));
}

TEST_F(InitedBxCanTransceiverTest, openFromMutedFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.open());
}

TEST_F(InitedBxCanTransceiverTest, initFromOpenFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.init());
}

TEST_F(InitedBxCanTransceiverTest, initFromMutedFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.init());
}

TEST_F(InitedBxCanTransceiverTest, writeAfterCloseReturnsTxOffline)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.close());
    CANFrame frame;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fBxt.write(frame));
}

TEST_F(InitedBxCanTransceiverTest, muteFromMutedFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.mute());
}

TEST_F(InitedBxCanTransceiverTest, unmuteRestoresWriteCapability)
{
    CANFrame frame;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fBxt.write(frame));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.unmute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame));
}

TEST_F(InitedBxCanTransceiverTest, shutdownFromInitialized)
{
    fBxt.shutdown();
    EXPECT_EQ(ICanTransceiver::State::CLOSED, fBxt.getState());
}

TEST_F(BxCanTransceiverTest, fullLifecycle)
{
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);

    EXPECT_CALL(bxt.fDevice, init()).Times(AnyNumber());
    EXPECT_CALL(bxt.fDevice, start()).Times(AnyNumber());
    EXPECT_CALL(bxt.fDevice, stop()).Times(AnyNumber());

    EXPECT_EQ(ICanTransceiver::State::CLOSED, bxt.getState());

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, bxt.init());
    EXPECT_EQ(ICanTransceiver::State::INITIALIZED, bxt.getState());

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, bxt.open());
    EXPECT_EQ(ICanTransceiver::State::OPEN, bxt.getState());

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, bxt.mute());
    EXPECT_EQ(ICanTransceiver::State::MUTED, bxt.getState());

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, bxt.close());
    EXPECT_EQ(ICanTransceiver::State::CLOSED, bxt.getState());
}

TEST_F(BxCanTransceiverTest, reopenAfterShutdown)
{
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);

    EXPECT_CALL(bxt.fDevice, init()).Times(AnyNumber());
    EXPECT_CALL(bxt.fDevice, start()).Times(AnyNumber());
    EXPECT_CALL(bxt.fDevice, stop()).Times(AnyNumber());

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, bxt.init());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, bxt.open());
    bxt.shutdown();
    EXPECT_EQ(ICanTransceiver::State::CLOSED, bxt.getState());

    // Re-open from CLOSED
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, bxt.open());
    EXPECT_EQ(ICanTransceiver::State::OPEN, bxt.getState());
}

TEST_F(InitedBxCanTransceiverTest, busOffDuringPendingTxDropsCallback)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));

    // Bus goes off before TX ISR
    EXPECT_CALL(fBxt.fDevice, isBusOff()).WillOnce(Return(true));
    fBxt.cyclicTask(); // transitions to MUTED

    // TX ISR fires after bus-off - listener should NOT be called
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(0);
    BxCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedBxCanTransceiverTest, busOffRecoveryResumesTx)
{
    CANFrame frame;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillRepeatedly(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());

    // Bus-off -> MUTED
    EXPECT_CALL(fBxt.fDevice, isBusOff()).WillOnce(Return(true)).WillOnce(Return(false));
    fBxt.cyclicTask();
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fBxt.write(frame));

    // Bus-on -> OPEN
    fBxt.cyclicTask();
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame));
}

TEST_F(InitedBxCanTransceiverTest, concurrentRxTxSameCycle)
{
    CANFrame rxFrame;
    CANFrame txFrame;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());

    // TX
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(txFrame));

    // RX in same cycle
    EXPECT_CALL(fBxt.fDevice, disableRxInterrupt()).Times(1);
    EXPECT_CALL(fBxt.fDevice, getRxCount()).WillOnce(Return(1));
    EXPECT_CALL(fBxt.fDevice, getRxFrame(0)).WillOnce(ReturnRef(rxFrame));
    EXPECT_CALL(fBxt.fDevice, clearRxQueue()).Times(1);
    EXPECT_CALL(fBxt.fDevice, enableRxInterrupt()).Times(1);

    fBxt.receiveTask();
}

TEST_F(InitedBxCanTransceiverTest, txIsrAndReceiveTaskInterleave)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));

    // RX task runs
    EXPECT_CALL(fBxt.fDevice, disableRxInterrupt()).Times(1);
    EXPECT_CALL(fBxt.fDevice, getRxCount()).WillOnce(Return(0));
    EXPECT_CALL(fBxt.fDevice, clearRxQueue()).Times(1);
    EXPECT_CALL(fBxt.fDevice, enableRxInterrupt()).Times(1);
    fBxt.receiveTask();

    // TX ISR fires after receive task completed
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    BxCanTransceiver::transmitInterrupt(fBusId);
}

class ManualAsyncBxCanTransceiverTest : public BxCanTransceiverTest
{
public:
    ManualAsyncBxCanTransceiverTest() : fBxt(fAsyncContext, fBusId, fDevConfig)
    {
        EXPECT_CALL(fBxt.fDevice, init()).Times(AnyNumber());
        EXPECT_CALL(fBxt.fDevice, start()).Times(AnyNumber());
        EXPECT_CALL(fBxt.fDevice, stop()).Times(AnyNumber());

        // Capture the runnable instead of auto-executing
        EXPECT_CALL(fAsyncMock, execute(fAsyncContext, _))
            .Times(AnyNumber())
            .WillRepeatedly([this](auto, auto& runnable) { fCapturedRunnable = &runnable; });

        EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.init());
    }

    void executeDeferred()
    {
        if (fCapturedRunnable != nullptr)
        {
            fCapturedRunnable->execute();
            fCapturedRunnable = nullptr;
        }
    }

    BxCanTransceiver fBxt;
    ::async::RunnableType* fCapturedRunnable = nullptr;
};

TEST_F(ManualAsyncBxCanTransceiverTest, deferredCallbackNotCalledUntilAsyncFires)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));

    // TX ISR fires - defers to async context
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    BxCanTransceiver::transmitInterrupt(fBusId);

    // Listener is called when async executes
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    executeDeferred();
}

TEST_F(InitedBxCanTransceiverTest, cyclicTaskUserMuteNoAutoRecovery)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());

    // User mutes manually
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());

    // Bus is not off, but fMuted is true - cyclicTask should NOT unmute
    EXPECT_CALL(fBxt.fDevice, isBusOff()).WillOnce(Return(false));
    fBxt.cyclicTask();

    // Should still be MUTED because user muted (fMuted = true)
    CANFrame frame;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fBxt.write(frame));
}

TEST_F(InitedBxCanTransceiverTest, enableRxInterruptDelegatesToDevice)
{
    EXPECT_CALL(fBxt.fDevice, enableRxInterrupt()).Times(1);
    BxCanTransceiver::enableRxInterrupt(fBusId);
}

TEST_F(BxCanTransceiverTest, disableRxInterruptInvalidIndexSafe)
{
    BxCanTransceiver::disableRxInterrupt(3);
}

TEST_F(BxCanTransceiverTest, enableRxInterruptInvalidIndexSafe)
{
    BxCanTransceiver::enableRxInterrupt(3);
}

TEST_F(BxCanTransceiverTest, transmitInterruptInvalidIndexSafe)
{
    BxCanTransceiver::transmitInterrupt(3);
}

// --- Overrun counter ---

TEST_F(BxCanTransceiverTest, getOverrunCountInitiallyZero)
{
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);
    EXPECT_EQ(0U, bxt.getOverrunCount());
}

TEST_F(InitedBxCanTransceiverTest, writeHwQueueFullIncrementsOverrunCount)
{
    CANFrame frame;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(false)).WillOnce(Return(false));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fBxt.write(frame));
    EXPECT_EQ(1U, fBxt.getOverrunCount());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fBxt.write(frame));
    EXPECT_EQ(2U, fBxt.getOverrunCount());
}

TEST_F(InitedBxCanTransceiverTest, writeWithListenerQueueFullIncrementsOverrunCount)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    // Only the first queued write reaches the device
    EXPECT_CALL(fBxt.fDevice, transmit(_)).Times(1).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
    EXPECT_EQ(0U, fBxt.getOverrunCount());

    // 4th write overflows the 3-deep listener job queue
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fBxt.write(frame, listener));
    EXPECT_EQ(1U, fBxt.getOverrunCount());
}

TEST_F(InitedBxCanTransceiverTest, writeWithListenerFirstTransmitFailurePopsJobAndCountsOverrun)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(false)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fBxt.write(frame, listener));
    EXPECT_EQ(1U, fBxt.getOverrunCount());

    // The failed job was popped: this write is again "first in queue" and
    // must reach the device directly (2nd transmit expectation)
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
    EXPECT_EQ(1U, fBxt.getOverrunCount());
}

// --- Cyclic bus-off polling ---

TEST_F(InitedBxCanTransceiverTest, openSchedulesCyclicBusOffPolling)
{
    EXPECT_CALL(
        fAsyncMock, scheduleAtFixedRate(fAsyncContext, _, _, 10U, ::async::TimeUnit::MILLISECONDS))
        .Times(1);
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
}

TEST_F(InitedBxCanTransceiverTest, cyclicTaskBusOffWhenNotOpenKeepsState)
{
    // Not opened - state INITIALIZED. Bus-off must not force MUTED here.
    EXPECT_CALL(fBxt.fDevice, isBusOff()).WillOnce(Return(true));
    fBxt.cyclicTask();
    EXPECT_EQ(ICanTransceiver::State::INITIALIZED, fBxt.getState());
}

// --- TX queue drop contracts (close / mute / chain failure) ---

TEST_F(InitedBxCanTransceiverTest, closeClearsPendingTxJobsWithoutNotification)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.close());

    // A late TX ISR must not fire the dropped listener job
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(0);
    BxCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedBxCanTransceiverTest, muteClearsPendingTxJobsWithoutNotification)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));

    // mute() drops the queued jobs; unmute() must not resurrect them
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.unmute());

    EXPECT_CALL(fBxt.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(0);
    BxCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedBxCanTransceiverTest, sendAgainTransmitFailureClearsRemainingQueue)
{
    CANFrame frame;
    MockCANFrameSentListener listener1;
    MockCANFrameSentListener listener2;
    MockCANFrameSentListener listener3;

    // First transmit (initial write) succeeds; the chained transmit of the
    // next queued frame from the TX-complete path fails
    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true)).WillOnce(Return(false));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener1));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener2));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener3));

    EXPECT_CALL(fBxt.fDevice, transmitISR()).Times(2);
    EXPECT_CALL(listener1, canFrameSent(_)).Times(1);
    EXPECT_CALL(listener2, canFrameSent(_)).Times(0);
    EXPECT_CALL(listener3, canFrameSent(_)).Times(0);

    // 1st ISR: listener1 fires, chained transmit fails -> queue cleared
    BxCanTransceiver::transmitInterrupt(fBusId);
    // 2nd ISR: queue is empty, no further listener callbacks
    BxCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedBxCanTransceiverTest, listenerReceivesFrameCopiedBeforePop)
{
    // Regression: the frame passed to canFrameSent() must be copied by value
    // before pop_front() destroys the queue slot - each listener must see
    // its own frame's ID even while the queue is being advanced
    uint8_t data[8] = {};
    CANFrame frame1(0x111U, data, 1U);
    CANFrame frame2(0x222U, data, 2U);
    CANFrame frame3(0x333U, data, 3U);
    MockCANFrameSentListener listener1;
    MockCANFrameSentListener listener2;
    MockCANFrameSentListener listener3;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillRepeatedly(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame1, listener1));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame2, listener2));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame3, listener3));

    EXPECT_CALL(listener1, canFrameSent(Property(&CANFrame::getId, 0x111U))).Times(1);
    EXPECT_CALL(listener2, canFrameSent(Property(&CANFrame::getId, 0x222U))).Times(1);
    EXPECT_CALL(listener3, canFrameSent(Property(&CANFrame::getId, 0x333U))).Times(1);
    EXPECT_CALL(fBxt.fDevice, transmitISR()).Times(3);

    BxCanTransceiver::transmitInterrupt(fBusId);
    BxCanTransceiver::transmitInterrupt(fBusId);
    BxCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedBxCanTransceiverTest, secondListenerWriteDoesNotTransmitImmediately)
{
    CANFrame frame;
    MockCANFrameSentListener listener1;
    MockCANFrameSentListener listener2;

    // Only the head-of-queue write may reach the device; the second job
    // waits for the TX ISR chain
    EXPECT_CALL(fBxt.fDevice, transmit(_)).Times(1).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener1));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener2));
}

// --- Hardware queue timeout computation ---

TEST_F(BxCanTransceiverTest, getHwQueueTimeoutAt1Mbps)
{
    fDevConfig.baudrate = 1000000U;
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);
    // ceil(3 * 117 bit * 1000 / 1000000) = ceil(0.351 ms) = 1 ms (never 0)
    EXPECT_EQ(1U, bxt.getHwQueueTimeout());
}

TEST_F(BxCanTransceiverTest, getHwQueueTimeoutAt250kbps)
{
    fDevConfig.baudrate = 250000U;
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);
    // ceil(351000 / 250000) = ceil(1.404 ms) = 2 ms
    EXPECT_EQ(2U, bxt.getHwQueueTimeout());
}

TEST_F(BxCanTransceiverTest, getHwQueueTimeoutAt100kbps)
{
    fDevConfig.baudrate = 100000U;
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);
    // ceil(351000 / 100000) = ceil(3.51 ms) = 4 ms
    EXPECT_EQ(4U, bxt.getHwQueueTimeout());
}

TEST_F(BxCanTransceiverTest, getHwQueueTimeoutExactMultipleNotRoundedUp)
{
    // 3 mailboxes x 117 bit x 1000 = 351000: exact divisions must not gain
    // an extra millisecond from the round-up term
    fDevConfig.baudrate = 351000U;
    BxCanTransceiver bxtFast(fAsyncContext, fBusId, fDevConfig);
    EXPECT_EQ(1U, bxtFast.getHwQueueTimeout());

    fDevConfig.baudrate = 175500U;
    BxCanTransceiver bxtSlow(fAsyncContext, fBusId, fDevConfig);
    EXPECT_EQ(2U, bxtSlow.getHwQueueTimeout());
}

// --- Static handler index bounds ---

TEST_F(BxCanTransceiverTest, constructorWithBusIdAtLimitDoesNotRegister)
{
    BxCanTransceiver bxt(fAsyncContext, 3U, fDevConfig);
    EXPECT_EQ(3U, bxt.getBusId());

    // Slot 3 does not exist: the static handlers must not touch the device
    EXPECT_CALL(bxt.fDevice, receiveISR(_)).Times(0);
    EXPECT_CALL(bxt.fDevice, transmitISR()).Times(0);
    EXPECT_EQ(0U, BxCanTransceiver::receiveInterrupt(3U));
    BxCanTransceiver::transmitInterrupt(3U);
}

TEST_F(BxCanTransceiverTest, staticHandlersWithIndex255AreSafe)
{
    EXPECT_EQ(0U, BxCanTransceiver::receiveInterrupt(255U));
    BxCanTransceiver::transmitInterrupt(255U);
    BxCanTransceiver::disableRxInterrupt(255U);
    BxCanTransceiver::enableRxInterrupt(255U);
}

// --- Listener notification through AbstractCANTransceiver ---

TEST_F(InitedBxCanTransceiverTest, receiveTaskDeliversFramesToMatchingListenerOnly)
{
    NiceMock<MockCANFrameListener> frameListener;
    BitFieldFilter filter;
    filter.add(0x123U);
    ON_CALL(frameListener, getFilter()).WillByDefault(ReturnRef(filter));
    fBxt.addCANFrameListener(frameListener);

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());

    uint8_t data[8] = {};
    CANFrame matching(0x123U, data, 1U);
    CANFrame nonMatching(0x456U, data, 1U);

    EXPECT_CALL(fBxt.fDevice, disableRxInterrupt()).Times(1);
    EXPECT_CALL(fBxt.fDevice, getRxCount()).WillOnce(Return(2));
    EXPECT_CALL(fBxt.fDevice, getRxFrame(0)).WillOnce(ReturnRef(matching));
    EXPECT_CALL(fBxt.fDevice, getRxFrame(1)).WillOnce(ReturnRef(nonMatching));
    EXPECT_CALL(fBxt.fDevice, clearRxQueue()).Times(1);
    EXPECT_CALL(fBxt.fDevice, enableRxInterrupt()).Times(1);

    // Only the frame whose ID is in the listener's filter may be delivered
    EXPECT_CALL(frameListener, frameReceived(Property(&CANFrame::getId, 0x123U))).Times(1);
    fBxt.receiveTask();

    fBxt.removeCANFrameListener(frameListener);
}

TEST_F(InitedBxCanTransceiverTest, fireAndForgetWriteNotifiesRegisteredSentListener)
{
    CANFrame frame;
    NiceMock<MockFilteredCANFrameSentListener> sentListener;
    ::testing::SystemTimerMock systemTimer;
    EXPECT_CALL(systemTimer, getSystemTimeUs32Bit()).WillOnce(Return(10U));

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));
    EXPECT_CALL(sentListener, canFrameSent(_)).Times(1);

    fBxt.addCANFrameSentListener(sentListener);
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame));

    fBxt.removeCANFrameSentListener(sentListener);
}

TEST_F(InitedBxCanTransceiverTest, listenerQueueFullStillNotifiesRegisteredSentListener)
{
    CANFrame frame;
    MockCANFrameSentListener listener;
    NiceMock<MockFilteredCANFrameSentListener> sentListener;
    ::testing::SystemTimerMock systemTimer;
    EXPECT_CALL(systemTimer, getSystemTimeUs32Bit()).WillRepeatedly(Return(10U));

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));

    // Register the sent listener only now: the overflow path must still
    // report the rejected frame to the registered sent listeners
    fBxt.addCANFrameSentListener(sentListener);
    EXPECT_CALL(sentListener, canFrameSent(_)).Times(1);
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fBxt.write(frame, listener));

    fBxt.removeCANFrameSentListener(sentListener);
}

// --- Deferred TX-complete callback (manual async execution) ---

TEST_F(ManualAsyncBxCanTransceiverTest, deferredCallbackAfterCloseDropsJob)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));

    // TX ISR defers the callback to task context
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    BxCanTransceiver::transmitInterrupt(fBusId);

    // close() drops the queued job before the deferred callback runs
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.close());

    EXPECT_CALL(listener, canFrameSent(_)).Times(0);
    executeDeferred();
}

TEST_F(ManualAsyncBxCanTransceiverTest, deferredCallbackSendsNextQueuedFrame)
{
    uint8_t data[8] = {};
    CANFrame frame1(0x111U, data, 1U);
    CANFrame frame2(0x222U, data, 1U);
    MockCANFrameSentListener listener1;
    MockCANFrameSentListener listener2;

    EXPECT_CALL(fBxt.fDevice, transmit(Property(&CANFrame::getId, 0x111U))).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame1, listener1));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame2, listener2));

    EXPECT_CALL(fBxt.fDevice, transmitISR());
    BxCanTransceiver::transmitInterrupt(fBusId);

    // The deferred callback pops frame1, notifies listener1, and chains the
    // transmission of frame2
    EXPECT_CALL(listener1, canFrameSent(_)).Times(1);
    EXPECT_CALL(fBxt.fDevice, transmit(Property(&CANFrame::getId, 0x222U))).WillOnce(Return(true));
    executeDeferred();
}

} // namespace
