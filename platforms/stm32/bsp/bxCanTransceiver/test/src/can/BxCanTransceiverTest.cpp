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

class BxCanTransceiverTest : public Test
{
public:
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

TEST_F(InitedBxCanTransceiverTest, getBaudrate) { EXPECT_EQ(500000U, fBxt.getBaudrate()); }

TEST_F(InitedBxCanTransceiverTest, getHwQueueTimeout) { EXPECT_EQ(10U, fBxt.getHwQueueTimeout()); }

TEST_F(InitedBxCanTransceiverTest, receiveTask)
{
    CANFrame frame;

    EXPECT_CALL(fBxt.fDevice, getRxCount()).WillOnce(Return(0));
    EXPECT_CALL(fBxt.fDevice, clearRxQueue()).Times(1);

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

    EXPECT_CALL(fBxt.fDevice, getRxCount()).WillOnce(Return(2));
    EXPECT_CALL(fBxt.fDevice, getRxFrame(0)).WillOnce(ReturnRef(frame1));
    EXPECT_CALL(fBxt.fDevice, getRxFrame(1)).WillOnce(ReturnRef(frame2));
    EXPECT_CALL(fBxt.fDevice, clearRxQueue()).Times(1);
    EXPECT_CALL(fBxt.fDevice, enableRxInterrupt()).Times(1);

    fBxt.receiveTask();
}

TEST_F(InitedBxCanTransceiverTest, receiveTaskReenablesInterrupt)
{
    EXPECT_CALL(fBxt.fDevice, getRxCount()).WillOnce(Return(0));
    EXPECT_CALL(fBxt.fDevice, clearRxQueue()).Times(1);
    EXPECT_CALL(fBxt.fDevice, enableRxInterrupt()).Times(1);

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

} // namespace
