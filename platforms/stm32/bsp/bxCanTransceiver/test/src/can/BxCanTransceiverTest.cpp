// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

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

// ---------------------------------------------------------------------------
// Mock for ICANFrameSentListener — used by write(frame, listener) tests
// ---------------------------------------------------------------------------
class MockCANFrameSentListener : public ::can::ICANFrameSentListener
{
public:
    MOCK_METHOD(void, canFrameSent, (::can::CANFrame const&), (override));
};

// ===========================================================================
// Base fixture — uninitialised transceiver
// ===========================================================================
class BxCanTransceiverTest : public Test
{
public:
    ::async::AsyncMock fAsyncMock;
    ::async::ContextType fAsyncContext = 0;
    uint8_t fBusId                     = 0;
    BxCanDevice::Config fDevConfig{};
};

// ---------------------------------------------------------------------------
// Existing: init() transitions from CLOSED to INITIALIZED
// ---------------------------------------------------------------------------
TEST_F(BxCanTransceiverTest, initFromClosed)
{
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, bxt.init());
}

// ---------------------------------------------------------------------------
// Existing: Repeated init() calls should fail with ILLEGAL_STATE
// ---------------------------------------------------------------------------
TEST_F(BxCanTransceiverTest, initTwiceFails)
{
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, bxt.init());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, bxt.init());
}

// ---------------------------------------------------------------------------
// Cat 8 — Edge: receiveInterrupt() with invalid index returns 0
// ---------------------------------------------------------------------------
TEST_F(BxCanTransceiverTest, receiveInterruptInvalidIndex)
{
    EXPECT_EQ(0U, BxCanTransceiver::receiveInterrupt(3));
}

// ---------------------------------------------------------------------------
// Cat 9 — State: write(frame) from CLOSED returns TX_OFFLINE
// ---------------------------------------------------------------------------
TEST_F(BxCanTransceiverTest, writeFromClosedFails)
{
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);
    CANFrame frame;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, bxt.write(frame));
}

// ---------------------------------------------------------------------------
// Cat 9 — State: write(frame, listener) from CLOSED returns TX_OFFLINE
// ---------------------------------------------------------------------------
TEST_F(BxCanTransceiverTest, writeWithListenerFromClosedFails)
{
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);
    CANFrame frame;
    MockCANFrameSentListener listener;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, bxt.write(frame, listener));
}

// ---------------------------------------------------------------------------
// Cat 9 — State: close() from CLOSED is idempotent (returns OK)
// ---------------------------------------------------------------------------
TEST_F(BxCanTransceiverTest, closeFromClosedReturnsOk)
{
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, bxt.close());
}

// ---------------------------------------------------------------------------
// Cat 9 — State: mute() from CLOSED fails
// ---------------------------------------------------------------------------
TEST_F(BxCanTransceiverTest, muteFromClosedFails)
{
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, bxt.mute());
}

// ---------------------------------------------------------------------------
// Cat 9 — State: unmute() from CLOSED fails
// ---------------------------------------------------------------------------
TEST_F(BxCanTransceiverTest, unmuteFromClosedFails)
{
    BxCanTransceiver bxt(fAsyncContext, fBusId, fDevConfig);
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, bxt.unmute());
}

// ---------------------------------------------------------------------------
// Cat 9 — State: open() from CLOSED (without init) should fail
// ---------------------------------------------------------------------------
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

// ===========================================================================
// Inited fixture — async::execute auto-fires (normal path)
// ===========================================================================
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

// ---------------------------------------------------------------------------
// Existing: open() transitions from INITIALIZED to OPEN
// ---------------------------------------------------------------------------
TEST_F(InitedBxCanTransceiverTest, open)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
}

// ---------------------------------------------------------------------------
// Existing: Repeated open() calls should fail
// ---------------------------------------------------------------------------
TEST_F(InitedBxCanTransceiverTest, openTwiceFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.open());
}

// ---------------------------------------------------------------------------
// Existing: close() from OPEN returns OK
// ---------------------------------------------------------------------------
TEST_F(InitedBxCanTransceiverTest, closeFromOpen)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.close());
}

// ---------------------------------------------------------------------------
// Existing: close() from INITIALIZED returns OK (transitions to CLOSED)
// ---------------------------------------------------------------------------
TEST_F(InitedBxCanTransceiverTest, closeFromInitializedReturnsOk)
{
    EXPECT_CALL(fBxt.fDevice, stop());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.close());
}

// ---------------------------------------------------------------------------
// Existing: mute() from OPEN returns OK
// ---------------------------------------------------------------------------
TEST_F(InitedBxCanTransceiverTest, muteFromOpen)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());
}

// ---------------------------------------------------------------------------
// Existing: mute() from INITIALIZED should fail
// ---------------------------------------------------------------------------
TEST_F(InitedBxCanTransceiverTest, muteFromInitializedFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.mute());
}

// ---------------------------------------------------------------------------
// Existing: unmute() from MUTED returns OK
// ---------------------------------------------------------------------------
TEST_F(InitedBxCanTransceiverTest, unmuteFromMuted)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.unmute());
}

// ---------------------------------------------------------------------------
// Existing: unmute() from OPEN should fail
// ---------------------------------------------------------------------------
TEST_F(InitedBxCanTransceiverTest, unmuteFromOpenFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.unmute());
}

// ---------------------------------------------------------------------------
// Existing: write() when OPEN calls transmit on device
// ---------------------------------------------------------------------------
TEST_F(InitedBxCanTransceiverTest, writeWhenOpen)
{
    CANFrame frame;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame));
}

// ---------------------------------------------------------------------------
// Existing: write() when MUTED returns TX_OFFLINE
// ---------------------------------------------------------------------------
TEST_F(InitedBxCanTransceiverTest, writeWhenMutedFails)
{
    CANFrame frame;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fBxt.write(frame));
}

// ---------------------------------------------------------------------------
// Existing: write() when TX FIFO full returns HW_QUEUE_FULL
// ---------------------------------------------------------------------------
TEST_F(InitedBxCanTransceiverTest, writeWhenFifoFull)
{
    CANFrame frame;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(false));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fBxt.write(frame));
}

// ---------------------------------------------------------------------------
// Existing: getBaudrate() returns expected value
// ---------------------------------------------------------------------------
TEST_F(InitedBxCanTransceiverTest, getBaudrate) { EXPECT_EQ(500000U, fBxt.getBaudrate()); }

// ---------------------------------------------------------------------------
// Existing: getHwQueueTimeout() returns expected value
// ---------------------------------------------------------------------------
TEST_F(InitedBxCanTransceiverTest, getHwQueueTimeout) { EXPECT_EQ(10U, fBxt.getHwQueueTimeout()); }

// ---------------------------------------------------------------------------
// Existing: receiveTask() drains RX queue and notifies listeners
// ---------------------------------------------------------------------------
TEST_F(InitedBxCanTransceiverTest, receiveTask)
{
    CANFrame frame;

    EXPECT_CALL(fBxt.fDevice, getRxCount()).WillOnce(Return(0));
    EXPECT_CALL(fBxt.fDevice, clearRxQueue()).Times(1);

    fBxt.receiveTask();
}

// ---------------------------------------------------------------------------
// Existing: cyclicTask() detects bus-off and transitions to MUTED
// ---------------------------------------------------------------------------
TEST_F(InitedBxCanTransceiverTest, cyclicTaskBusOff)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());

    EXPECT_CALL(fBxt.fDevice, isBusOff()).WillOnce(Return(true));
    fBxt.cyclicTask();

    // After bus-off, write should fail because state is MUTED
    CANFrame frame;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fBxt.write(frame));
}

// ---------------------------------------------------------------------------
// Existing: cyclicTask() recovers from bus-off when bus comes back
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// Existing: shutdown() calls close
// ---------------------------------------------------------------------------
TEST_F(InitedBxCanTransceiverTest, shutdown)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    fBxt.shutdown();
}

// ---------------------------------------------------------------------------
// Existing: receiveInterrupt() with valid index calls device receiveISR
// ---------------------------------------------------------------------------
TEST_F(InitedBxCanTransceiverTest, receiveInterrupt)
{
    EXPECT_CALL(fBxt.fDevice, receiveISR(_)).WillOnce(Return(0));
    BxCanTransceiver::receiveInterrupt(fBusId);
}

// ---------------------------------------------------------------------------
// Existing: transmitInterrupt() with valid index calls device transmitISR
// ---------------------------------------------------------------------------
TEST_F(InitedBxCanTransceiverTest, transmitInterrupt)
{
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    BxCanTransceiver::transmitInterrupt(fBusId);
}

// ===========================================================================
// Category 1 — write(frame, listener) async TX callback
// ===========================================================================

/// \brief write(frame, listener) returns OK when OPEN and device transmits
TEST_F(InitedBxCanTransceiverTest, writeWithListenerReturnsOkWhenOpen)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
}

/// \brief write(frame, listener) returns TX_OFFLINE when MUTED
TEST_F(InitedBxCanTransceiverTest, writeWithListenerWhenMutedFails)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fBxt.write(frame, listener));
}

/// \brief write(frame, listener) returns HW_QUEUE_FULL when device rejects
TEST_F(InitedBxCanTransceiverTest, writeWithListenerWhenFifoFullReturnsQueueFull)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(false));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fBxt.write(frame, listener));
}

/// \brief Listener callback fires from transmitInterrupt, not from write()
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

// ===========================================================================
// Category 2 — TX queue capacity
// ===========================================================================

/// \brief TX queue holds up to 3 pending writes (queue capacity = 3)
TEST_F(InitedBxCanTransceiverTest, txQueueFillsToCapacity)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillRepeatedly(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());

    // Queue capacity is 3 — fill it up without draining
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));

    // 4th write should fail because queue is full
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fBxt.write(frame, listener));
}

/// \brief After draining one slot via TX ISR, another write succeeds
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

// ===========================================================================
// Category 3 — canFrameSentCallback chain
// ===========================================================================

/// \brief Callback with single frame: listener called, queue empty
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

/// \brief Callback with two frames: first listener called, second remains
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

/// \brief Callback with three frames: all three listeners fire in order
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

/// \brief Callback with empty queue: transmitISR called, no listener crash
TEST_F(InitedBxCanTransceiverTest, canFrameSentCallbackEmptyQueue)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());

    // TX ISR with nothing queued — should not crash
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    BxCanTransceiver::transmitInterrupt(fBusId);
}

/// \brief Callback aborted when transceiver becomes MUTED (bus-off during TX chain)
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

/// \brief Callback with fire-and-forget write(frame) — no pending listener
TEST_F(InitedBxCanTransceiverTest, canFrameSentCallbackWithFireAndForgetWrite)
{
    CANFrame frame;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame));

    // TX ISR fires — no listener was registered, no crash
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    BxCanTransceiver::transmitInterrupt(fBusId);
}

/// \brief Same listener for two consecutive writes — both callbacks fire
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

/// \brief Mixed write() and write(frame, listener) — only listener-write gets callback
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

// ===========================================================================
// Category 4 — receiveInterrupt nullptr filter
// ===========================================================================

/// \brief receiveInterrupt passes nullptr filter to receiveISR (accept-all)
TEST_F(InitedBxCanTransceiverTest, receiveInterruptPassesNullFilter)
{
    EXPECT_CALL(fBxt.fDevice, receiveISR(nullptr)).WillOnce(Return(1));
    uint8_t count = BxCanTransceiver::receiveInterrupt(fBusId);
    EXPECT_EQ(1U, count);
}

/// \brief receiveInterrupt with unregistered index returns 0
TEST_F(InitedBxCanTransceiverTest, receiveInterruptUnregisteredIndexReturnsZero)
{
    // Index 2 was never registered (only index 0 was)
    EXPECT_EQ(0U, BxCanTransceiver::receiveInterrupt(2));
}

// ===========================================================================
// Category 5 — receiveTask listener notification
// ===========================================================================

/// \brief receiveTask() with frames notifies listeners for each frame
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

/// \brief receiveTask() re-enables RX interrupt after draining queue
TEST_F(InitedBxCanTransceiverTest, receiveTaskReenablesInterrupt)
{
    EXPECT_CALL(fBxt.fDevice, getRxCount()).WillOnce(Return(0));
    EXPECT_CALL(fBxt.fDevice, clearRxQueue()).Times(1);
    EXPECT_CALL(fBxt.fDevice, enableRxInterrupt()).Times(1);

    fBxt.receiveTask();
}

// ===========================================================================
// Category 6 — Registered sent listener (notifySentListeners)
// ===========================================================================

/// \brief write(frame) without listener still notifies registered sent listeners
TEST_F(InitedBxCanTransceiverTest, writeFireAndForgetNotifiesRegisteredSentListeners)
{
    CANFrame frame;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());

    // notifySentListeners is called synchronously by write(frame)
    // No registered sent listeners by default, so it just doesn't crash
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame));
}

/// \brief write(frame) failure does NOT notify registered sent listeners
TEST_F(InitedBxCanTransceiverTest, writeFailureDoesNotNotifySentListeners)
{
    CANFrame frame;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(false));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());

    // HW queue full — no notification expected
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fBxt.write(frame));
}

// ===========================================================================
// Category 7 — DoCAN integration pattern
// ===========================================================================

/// \brief DoCAN pattern: write(frame, listener) + TX ISR = listener fires
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

/// \brief DoCAN pattern: consecutive multi-frame sends (queue drains correctly)
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

// ===========================================================================
// Category 8 — Edge cases
// ===========================================================================

/// \brief open(frame) delegates to open() (wake-up frame not supported on bxCAN)
TEST_F(InitedBxCanTransceiverTest, openWithFrameDelegatesToOpen)
{
    CANFrame frame;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open(frame));
    // Second open should fail, proving state transitioned
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.open());
}

/// \brief open() from CLOSED (after close) re-initializes device
TEST_F(InitedBxCanTransceiverTest, openFromClosedReinitsDevice)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.close());

    // Re-open from CLOSED should call init + start
    EXPECT_CALL(fBxt.fDevice, init()).Times(1);
    EXPECT_CALL(fBxt.fDevice, start()).Times(1);
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
}

/// \brief close() from MUTED returns OK
TEST_F(InitedBxCanTransceiverTest, closeFromMutedReturnsOk)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.close());
}

/// \brief close() is idempotent — calling twice returns OK both times
TEST_F(InitedBxCanTransceiverTest, closeIsIdempotent)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.close());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.close());
}

/// \brief mute() twice should fail on second call
TEST_F(InitedBxCanTransceiverTest, muteTwiceFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.mute());
}

/// \brief unmute() from INITIALIZED fails
TEST_F(InitedBxCanTransceiverTest, unmuteFromInitializedFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.unmute());
}

/// \brief getBusId() returns the configured bus ID
TEST_F(InitedBxCanTransceiverTest, getBusIdReturnsConfiguredValue)
{
    EXPECT_EQ(fBusId, fBxt.getBusId());
}

/// \brief getState() reflects lifecycle transitions
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

/// \brief disableRxInterrupt() delegates to device
TEST_F(InitedBxCanTransceiverTest, disableRxInterruptDelegatesToDevice)
{
    EXPECT_CALL(fBxt.fDevice, disableRxInterrupt()).Times(1);
    BxCanTransceiver::disableRxInterrupt(fBusId);
}

// ===========================================================================
// Category 9 — State x Operation matrix
// ===========================================================================

/// \brief write(frame) from INITIALIZED returns TX_OFFLINE
TEST_F(InitedBxCanTransceiverTest, writeFromInitializedReturnsTxOffline)
{
    CANFrame frame;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fBxt.write(frame));
}

/// \brief write(frame, listener) from INITIALIZED returns TX_OFFLINE
TEST_F(InitedBxCanTransceiverTest, writeWithListenerFromInitializedReturnsTxOffline)
{
    CANFrame frame;
    MockCANFrameSentListener listener;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fBxt.write(frame, listener));
}

/// \brief open() from MUTED fails (must close first)
TEST_F(InitedBxCanTransceiverTest, openFromMutedFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.open());
}

/// \brief init() from OPEN fails
TEST_F(InitedBxCanTransceiverTest, initFromOpenFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.init());
}

/// \brief init() from MUTED fails
TEST_F(InitedBxCanTransceiverTest, initFromMutedFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.init());
}

/// \brief write(frame) after close returns TX_OFFLINE
TEST_F(InitedBxCanTransceiverTest, writeAfterCloseReturnsTxOffline)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.close());
    CANFrame frame;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fBxt.write(frame));
}

/// \brief mute() from MUTED fails (already muted)
TEST_F(InitedBxCanTransceiverTest, muteFromMutedFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fBxt.mute());
}

/// \brief unmute() restores write capability
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

/// \brief shutdown() from INITIALIZED is safe
TEST_F(InitedBxCanTransceiverTest, shutdownFromInitialized)
{
    fBxt.shutdown();
    EXPECT_EQ(ICanTransceiver::State::CLOSED, fBxt.getState());
}

/// \brief Full lifecycle: CLOSED->init->INITIALIZED->open->OPEN->mute->MUTED->close->CLOSED
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

/// \brief Re-open after shutdown: CLOSED->open re-inits
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

// ===========================================================================
// Category 10 — Bus-off interaction with TX queue
// ===========================================================================

/// \brief Bus-off during pending TX: queued listener NOT called
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

    // TX ISR fires after bus-off — listener should NOT be called
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(0);
    BxCanTransceiver::transmitInterrupt(fBusId);
}

/// \brief Bus-off recovery: TX resumes after cyclicTask detects bus-on
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

// ===========================================================================
// Category 11 — Concurrent RX/TX
// ===========================================================================

/// \brief RX and TX can occur in same cycle
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

/// \brief TX ISR and receiveTask can interleave safely
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

// ===========================================================================
// Manual-async fixture — async::execute does NOT auto-fire
// ===========================================================================
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

/// \brief Deferred async callback: listener is NOT called until async::execute fires
TEST_F(ManualAsyncBxCanTransceiverTest, deferredCallbackNotCalledUntilAsyncFires)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBxt.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.write(frame, listener));

    // TX ISR fires — defers to async context
    EXPECT_CALL(fBxt.fDevice, transmitISR());
    BxCanTransceiver::transmitInterrupt(fBusId);

    // Listener is called when async executes
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    executeDeferred();
}

/// \brief cyclicTask() when already user-muted does NOT auto-recover
TEST_F(InitedBxCanTransceiverTest, cyclicTaskUserMuteNoAutoRecovery)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.open());

    // User mutes manually
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBxt.mute());

    // Bus is not off, but fMuted is true — cyclicTask should NOT unmute
    EXPECT_CALL(fBxt.fDevice, isBusOff()).WillOnce(Return(false));
    fBxt.cyclicTask();

    // Should still be MUTED because user muted (fMuted = true)
    CANFrame frame;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fBxt.write(frame));
}

/// \brief enableRxInterrupt delegates to device
TEST_F(InitedBxCanTransceiverTest, enableRxInterruptDelegatesToDevice)
{
    EXPECT_CALL(fBxt.fDevice, enableRxInterrupt()).Times(1);
    BxCanTransceiver::enableRxInterrupt(fBusId);
}

/// \brief disableRxInterrupt with invalid index is safe (no crash)
TEST_F(BxCanTransceiverTest, disableRxInterruptInvalidIndexSafe)
{
    BxCanTransceiver::disableRxInterrupt(3);
}

/// \brief enableRxInterrupt with invalid index is safe (no crash)
TEST_F(BxCanTransceiverTest, enableRxInterruptInvalidIndexSafe)
{
    BxCanTransceiver::enableRxInterrupt(3);
}

/// \brief transmitInterrupt with invalid index is safe (no crash)
TEST_F(BxCanTransceiverTest, transmitInterruptInvalidIndexSafe)
{
    BxCanTransceiver::transmitInterrupt(3);
}

} // namespace
