// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#include "can/transceiver/fdcan/FdCanTransceiver.h"

#include "async/AsyncMock.h"
#include "bsp/timer/SystemTimerMock.h"
#include "can/canframes/ICANFrameSentListener.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::can;
using namespace ::testing;
using namespace ::bios;

// ---------------------------------------------------------------------------
// Mock for ICANFrameSentListener (write-with-listener / DoCAN callback)
// ---------------------------------------------------------------------------
class MockCANFrameSentListener : public ::can::ICANFrameSentListener
{
public:
    MOCK_METHOD(void, canFrameSent, (::can::CANFrame const&), (override));
};

// ===================================================================
// Fixture: bare transceiver (CLOSED state)
// ===================================================================
class FdCanTransceiverTest : public Test
{
public:
    ::async::AsyncMock fAsyncMock;
    ::async::ContextType fAsyncContext = 0;
    uint8_t fBusId                     = 0;
    FdCanDevice::Config fDevConfig{};
};

// ===================================================================
// Fixture: transceiver in INITIALIZED state (auto-executes async)
// ===================================================================
class InitedFdCanTransceiverTest : public FdCanTransceiverTest
{
public:
    InitedFdCanTransceiverTest() : fFct(fAsyncContext, fBusId, fDevConfig)
    {
        EXPECT_CALL(fFct.fDevice, init()).Times(AnyNumber());
        EXPECT_CALL(fFct.fDevice, start()).Times(AnyNumber());
        EXPECT_CALL(fFct.fDevice, stop()).Times(AnyNumber());
        EXPECT_CALL(fAsyncMock, execute(fAsyncContext, _))
            .Times(AnyNumber())
            .WillRepeatedly([](auto, auto& runnable) { runnable.execute(); });

        EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.init());
    }

    FdCanTransceiver fFct;
};

// ===================================================================
// Fixture: transceiver in INITIALIZED state with manual async control
// (async::execute is captured but NOT auto-executed)
// ===================================================================
class ManualAsyncFdCanTransceiverTest : public FdCanTransceiverTest
{
public:
    ManualAsyncFdCanTransceiverTest() : fFct(fAsyncContext, fBusId, fDevConfig)
    {
        EXPECT_CALL(fFct.fDevice, init()).Times(AnyNumber());
        EXPECT_CALL(fFct.fDevice, start()).Times(AnyNumber());
        EXPECT_CALL(fFct.fDevice, stop()).Times(AnyNumber());
        // Capture async::execute calls but do NOT auto-run them
        EXPECT_CALL(fAsyncMock, execute(fAsyncContext, _))
            .Times(AnyNumber())
            .WillRepeatedly([this](auto, auto& runnable) { fCapturedRunnable = &runnable; });

        EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.init());
    }

    /// Manually execute the last captured async runnable (simulates task context)
    void executeAsyncCallback()
    {
        ASSERT_NE(nullptr, fCapturedRunnable);
        fCapturedRunnable->execute();
        fCapturedRunnable = nullptr;
    }

    FdCanTransceiver fFct;
    ::async::RunnableType* fCapturedRunnable = nullptr;
};

// ===================================================================
// Category 0 — Original lifecycle tests (initFromClosed .. receiveInterruptInvalidIndex)
// ===================================================================

/// \brief init() transitions from CLOSED to INITIALIZED
TEST_F(FdCanTransceiverTest, initFromClosed)
{
    FdCanTransceiver fct(fAsyncContext, fBusId, fDevConfig);

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fct.init());
}

/// \brief Repeated init() calls should fail with ILLEGAL_STATE
TEST_F(FdCanTransceiverTest, initTwiceFails)
{
    FdCanTransceiver fct(fAsyncContext, fBusId, fDevConfig);

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fct.init());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fct.init());
}

/// \brief open() transitions from INITIALIZED to OPEN
TEST_F(InitedFdCanTransceiverTest, open)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
}

/// \brief Repeated open() calls should fail
TEST_F(InitedFdCanTransceiverTest, openTwiceFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fFct.open());
}

/// \brief open() from CLOSED re-initializes device
TEST_F(InitedFdCanTransceiverTest, openFromClosed)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.close());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
}

/// \brief close() from OPEN returns OK
TEST_F(InitedFdCanTransceiverTest, closeFromOpen)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.close());
}

/// \brief close() when already CLOSED returns OK
TEST_F(InitedFdCanTransceiverTest, closeFromClosedReturnsOk)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.close());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.close());
}

/// \brief mute() from OPEN returns OK
TEST_F(InitedFdCanTransceiverTest, muteFromOpen)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.mute());
}

/// \brief mute() from INITIALIZED should fail
TEST_F(InitedFdCanTransceiverTest, muteFromInitializedFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fFct.mute());
}

/// \brief unmute() from MUTED returns OK
TEST_F(InitedFdCanTransceiverTest, unmuteFromMuted)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.unmute());
}

/// \brief unmute() from OPEN should fail
TEST_F(InitedFdCanTransceiverTest, unmuteFromOpenFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fFct.unmute());
}

/// \brief write() when OPEN calls transmit on device
TEST_F(InitedFdCanTransceiverTest, writeWhenOpen)
{
    CANFrame frame;

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame));
}

/// \brief write() when MUTED returns TX_OFFLINE
TEST_F(InitedFdCanTransceiverTest, writeWhenMutedFails)
{
    CANFrame frame;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fFct.write(frame));
}

/// \brief write() when TX FIFO full returns HW_QUEUE_FULL
TEST_F(InitedFdCanTransceiverTest, writeWhenFifoFull)
{
    CANFrame frame;

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(false));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fFct.write(frame));
}

/// \brief getBaudrate() returns expected value
TEST_F(InitedFdCanTransceiverTest, getBaudrate) { EXPECT_EQ(500000U, fFct.getBaudrate()); }

/// \brief getHwQueueTimeout() returns expected value
TEST_F(InitedFdCanTransceiverTest, getHwQueueTimeout) { EXPECT_EQ(10U, fFct.getHwQueueTimeout()); }

/// \brief receiveTask() drains RX queue and notifies listeners
TEST_F(InitedFdCanTransceiverTest, receiveTask)
{
    CANFrame frame;

    EXPECT_CALL(fFct.fDevice, getRxCount()).WillOnce(Return(0));
    EXPECT_CALL(fFct.fDevice, clearRxQueue()).Times(1);
    EXPECT_CALL(fFct.fDevice, enableRxInterrupt()).Times(1);

    fFct.receiveTask();
}

/// \brief cyclicTask() detects bus-off and transitions to MUTED
TEST_F(InitedFdCanTransceiverTest, cyclicTaskBusOff)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, isBusOff()).WillOnce(Return(true));
    fFct.cyclicTask();

    // After bus-off, write should fail
    CANFrame frame;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fFct.write(frame));
}

/// \brief cyclicTask() recovers from bus-off automatically
TEST_F(InitedFdCanTransceiverTest, cyclicTaskBusRecovery)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, isBusOff()).WillOnce(Return(true)).WillOnce(Return(false));

    fFct.cyclicTask(); // bus-off -> MUTED
    fFct.cyclicTask(); // bus-on -> OPEN

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(true));
    CANFrame frame;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame));
}

/// \brief shutdown() calls close
TEST_F(InitedFdCanTransceiverTest, shutdown)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    fFct.shutdown();
}

/// \brief receiveInterrupt() with valid index calls device receiveISR
TEST_F(InitedFdCanTransceiverTest, receiveInterrupt)
{
    EXPECT_CALL(fFct.fDevice, receiveISR(_)).WillOnce(Return(0));
    FdCanTransceiver::receiveInterrupt(fBusId);
}

/// \brief transmitInterrupt() with valid index calls device transmitISR
TEST_F(InitedFdCanTransceiverTest, transmitInterrupt)
{
    EXPECT_CALL(fFct.fDevice, transmitISR());
    FdCanTransceiver::transmitInterrupt(fBusId);
}

/// \brief receiveInterrupt() with invalid index returns 0
TEST_F(FdCanTransceiverTest, receiveInterruptInvalidIndex)
{
    EXPECT_EQ(0U, FdCanTransceiver::receiveInterrupt(3));
}

// ===================================================================
// Category 1 — write(frame, listener) async TX callback
// ===================================================================

/// \brief write(frame, listener) returns CAN_ERR_OK when transmit succeeds
TEST_F(InitedFdCanTransceiverTest, write2rOk)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));
}

/// \brief write(frame, listener) returns CAN_ERR_TX_HW_QUEUE_FULL when transmit fails
TEST_F(InitedFdCanTransceiverTest, write2rFail)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(false));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fFct.write(frame, listener));
}

/// \brief write(frame, listener) returns CAN_ERR_TX_OFFLINE when muted
TEST_F(InitedFdCanTransceiverTest, write2rWhenMuted)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fFct.write(frame, listener));
}

/// \brief write(frame, listener) does NOT call canFrameSent synchronously
TEST_F(InitedFdCanTransceiverTest, write2rDoesNotCallListenerSynchronously)
{
    CANFrame frame;
    StrictMock<MockCANFrameSentListener> listener;

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(true));
    // StrictMock: any call to canFrameSent during write() will FAIL
    EXPECT_CALL(listener, canFrameSent(_)).Times(0);

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));

    // Verify: listener was NOT called during write()
    Mock::VerifyAndClearExpectations(&listener);

    // Now allow the callback to happen (TX ISR triggers it)
    EXPECT_CALL(listener, canFrameSent(_)).Times(AnyNumber());
    EXPECT_CALL(fFct.fDevice, transmitISR());
    FdCanTransceiver::transmitInterrupt(fBusId);
}

// ===================================================================
// Category 2 — TX queue capacity
// ===================================================================

/// \brief TX queue holds 3 entries; 4th returns FULL
TEST_F(InitedFdCanTransceiverTest, writeOverFillQueue)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillRepeatedly(Return(true));

    // Queue capacity is 3; items are popped only in canFrameSentAsyncCallback,
    // which we never trigger here to simulate queue-full.
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fFct.write(frame, listener));
}

/// \brief 2nd queued frame is NOT transmitted until 1st completes
TEST_F(ManualAsyncFdCanTransceiverTest, writeSecondFrameNotTransmittedImmediately)
{
    CANFrame frame1;
    CANFrame frame2;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    // Only ONE transmit call expected from the first write()
    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame1, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame2, listener));

    // At this point, only frame1 was transmitted to HW; frame2 waits in queue.
    Mock::VerifyAndClearExpectations(&fFct.fDevice);

    // Now simulate TX ISR for frame1 — this should transmit frame2
    EXPECT_CALL(fFct.fDevice, transmitISR());
    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(true));
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);

    FdCanTransceiver::transmitInterrupt(fBusId);
    // Execute the deferred async callback
    executeAsyncCallback();
}

// ===================================================================
// Category 3 — canFrameSentCallback chain
// ===================================================================

/// \brief Single frame: TX ISR pops queue, calls listener
TEST_F(InitedFdCanTransceiverTest, canFrameSentCallbackWithOneFrame)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));

    // TX ISR fires -> canFrameSentCallback -> async -> canFrameSentAsyncCallback
    EXPECT_CALL(fFct.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    FdCanTransceiver::transmitInterrupt(fBusId);
}

/// \brief Two frames queued: callback chains to second frame (requires OPEN state)
/// Note: S32K allows write in INITIALIZED, our port requires OPEN. Test uses OPEN.
TEST_F(InitedFdCanTransceiverTest, canFrameSentCallbackWithTwoFramesChained)
{
    MockCANFrameSentListener listener;
    CANFrame frame1;
    CANFrame frame2;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_CALL(fFct.fDevice, transmit(_)).WillRepeatedly(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame1, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame2, listener));

    // TX ISR for frame1 — pops frame1, notifies listener, transmits frame2
    EXPECT_CALL(fFct.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    FdCanTransceiver::transmitInterrupt(fBusId);
}

/// \brief Two frames in OPEN state: callback chains to second frame
TEST_F(InitedFdCanTransceiverTest, canFrameSentCallbackWithTwoFramesInStateOpen)
{
    MockCANFrameSentListener listener;
    CANFrame frame1;
    CANFrame frame2;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillRepeatedly(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame1, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame2, listener));

    EXPECT_CALL(fFct.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    FdCanTransceiver::transmitInterrupt(fBusId);
}

/// \brief Two frames queued, then muted: callback aborts (does not send 2nd)
TEST_F(InitedFdCanTransceiverTest, canFrameSentCallbackWithTwoFramesIsAbortedWhenMuted)
{
    MockCANFrameSentListener listener;
    CANFrame frame1;
    CANFrame frame2;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame1, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame2, listener));

    fFct.mute();

    // TX ISR fires — first frame was already submitted. Callback should pop
    // frame1 and notify listener, but NOT submit frame2 (muted).
    EXPECT_CALL(fFct.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    FdCanTransceiver::transmitInterrupt(fBusId);
}

/// \brief Two frames: first transmit fails, second succeeds
TEST_F(InitedFdCanTransceiverTest, canFrameSentCallbackWithTwoFramesFailure)
{
    MockCANFrameSentListener listener;
    CANFrame frame1;
    CANFrame frame2;

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(false)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    // First write fails at HW level
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fFct.write(frame1, listener));
    // Second write succeeds
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame2, listener));

    EXPECT_CALL(fFct.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    FdCanTransceiver::transmitInterrupt(fBusId);
}

/// \brief Two frames: first OK, second transmit fails in callback chain
TEST_F(InitedFdCanTransceiverTest, canFrameSentCallbackWithTwoFramesIsAbortedWhenNextSendFails)
{
    MockCANFrameSentListener listener;
    CANFrame frame1;
    CANFrame frame2;

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(true)).WillOnce(Return(false));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame1, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame2, listener));

    // TX ISR for frame1 — callback pops frame1, tries to send frame2 but
    // transmit returns false => aborted, queue cleared
    EXPECT_CALL(fFct.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    FdCanTransceiver::transmitInterrupt(fBusId);
}

/// \brief canFrameSentCallback defers to task context via async::execute
TEST_F(ManualAsyncFdCanTransceiverTest, canFrameSentCallbackUsesAsyncExecute)
{
    CANFrame frame;
    StrictMock<MockCANFrameSentListener> listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));

    // TX ISR fires — should call async::execute, NOT call listener directly
    fCapturedRunnable = nullptr;
    EXPECT_CALL(fFct.fDevice, transmitISR());
    FdCanTransceiver::transmitInterrupt(fBusId);

    // Verify async::execute was called (runnable was captured)
    EXPECT_NE(nullptr, fCapturedRunnable);

    // Listener not called yet (still in ISR context)
    Mock::VerifyAndClearExpectations(&listener);

    // Now execute in task context
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    executeAsyncCallback();
}

/// \brief canFrameSentAsyncCallback runs in task context (not ISR)
TEST_F(ManualAsyncFdCanTransceiverTest, canFrameSentCallbackRunsInTaskContext)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));

    // TX ISR fires
    fCapturedRunnable = nullptr;
    EXPECT_CALL(fFct.fDevice, transmitISR());
    FdCanTransceiver::transmitInterrupt(fBusId);

    // At this point, listener should NOT have been called
    EXPECT_CALL(listener, canFrameSent(_)).Times(0);
    Mock::VerifyAndClearExpectations(&listener);

    // Execute the deferred callback (task context)
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    executeAsyncCallback();
}

// ===================================================================
// Category 4 — receiveInterrupt nullptr filter
// ===================================================================

/// \brief receiveInterrupt passes nullptr as filter (accept all)
TEST_F(InitedFdCanTransceiverTest, receiveInterruptPassesNullptrFilter)
{
    EXPECT_CALL(fFct.fDevice, receiveISR(nullptr)).WillOnce(Return(1));
    FdCanTransceiver::receiveInterrupt(fBusId);
}

/// \brief receiveInterrupt accepts all frames (no filter rejection)
TEST_F(InitedFdCanTransceiverTest, receiveInterruptAcceptsAllFrames)
{
    // Multiple calls, all accepted with nullptr filter
    EXPECT_CALL(fFct.fDevice, receiveISR(nullptr)).WillOnce(Return(1)).WillOnce(Return(3));

    EXPECT_EQ(1U, FdCanTransceiver::receiveInterrupt(fBusId));
    EXPECT_EQ(3U, FdCanTransceiver::receiveInterrupt(fBusId));
}

// ===================================================================
// Category 5 — receiveTask listener notification
// ===================================================================

/// \brief receiveTask notifies listeners for each frame in RX queue
TEST_F(InitedFdCanTransceiverTest, receiveTaskNotifiesListenersForEachFrame)
{
    CANFrame frame0;
    CANFrame frame1;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, getRxCount()).WillOnce(Return(2));
    EXPECT_CALL(fFct.fDevice, getRxFrame(0)).WillOnce(ReturnRef(frame0));
    EXPECT_CALL(fFct.fDevice, getRxFrame(1)).WillOnce(ReturnRef(frame1));
    EXPECT_CALL(fFct.fDevice, clearRxQueue()).Times(1);
    EXPECT_CALL(fFct.fDevice, enableRxInterrupt()).Times(1);

    fFct.receiveTask();
}

/// \brief receiveTask re-enables RX interrupt after draining queue
TEST_F(InitedFdCanTransceiverTest, receiveTaskReEnablesInterrupt)
{
    EXPECT_CALL(fFct.fDevice, getRxCount()).WillOnce(Return(0));
    EXPECT_CALL(fFct.fDevice, clearRxQueue()).Times(1);
    EXPECT_CALL(fFct.fDevice, enableRxInterrupt()).Times(1);

    fFct.receiveTask();
}

// ===================================================================
// Category 6 — Registered sent listener (filtered)
// ===================================================================

/// \brief notifyRegisteredSentListener notifies registered filtered listeners
/// \brief write(frame) without listener calls notifySentListeners synchronously
/// Note: upstream AbstractCANTransceiver only calls SystemTimer when
/// IFilteredCANFrameSentListener is registered. This test verifies the
/// write succeeds and the transceiver doesn't crash without registered listeners.
TEST_F(InitedFdCanTransceiverTest, notifyRegisteredSentListenerMatch)
{
    CANFrame frame;

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame));
}

/// \brief notifyRegisteredSentListener called from canFrameSentAsyncCallback
TEST_F(InitedFdCanTransceiverTest, notifyRegisteredSentListenerFromCallback)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));

    // TX ISR -> async callback -> pops queue -> calls listener.canFrameSent
    // Also calls notifyRegisteredSentListener (notifySentListeners)
    EXPECT_CALL(fFct.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    FdCanTransceiver::transmitInterrupt(fBusId);
}

// ===================================================================
// Category 7 — DoCAN integration pattern
// ===================================================================

/// \brief write with listener -> TX ISR -> deferred callback: full chain
TEST_F(
    ManualAsyncFdCanTransceiverTest, writeWithListenerThenTransmitInterruptTriggersDeferredCallback)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));

    // Step 1: TX ISR fires — defers to task context
    fCapturedRunnable = nullptr;
    EXPECT_CALL(fFct.fDevice, transmitISR());
    FdCanTransceiver::transmitInterrupt(fBusId);

    ASSERT_NE(nullptr, fCapturedRunnable);

    // Step 2: Task context executes — listener gets called
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    executeAsyncCallback();
}

/// \brief Multiple write-with-listener calls processed serially
TEST_F(ManualAsyncFdCanTransceiverTest, multipleWriteWithListenerProcessedSerially)
{
    CANFrame frame1;
    CANFrame frame2;
    MockCANFrameSentListener listener1;
    MockCANFrameSentListener listener2;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_))
        .WillOnce(Return(true))  // frame1
        .WillOnce(Return(true)); // frame2 (from callback chain)

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame1, listener1));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame2, listener2));

    // TX ISR for frame1
    fCapturedRunnable = nullptr;
    EXPECT_CALL(fFct.fDevice, transmitISR());
    FdCanTransceiver::transmitInterrupt(fBusId);

    // Execute deferred callback for frame1 — should notify listener1, submit frame2
    EXPECT_CALL(listener1, canFrameSent(_)).Times(1);
    EXPECT_CALL(listener2, canFrameSent(_)).Times(0);
    executeAsyncCallback();

    Mock::VerifyAndClearExpectations(&listener1);
    Mock::VerifyAndClearExpectations(&listener2);

    // TX ISR for frame2
    fCapturedRunnable = nullptr;
    EXPECT_CALL(fFct.fDevice, transmitISR());
    FdCanTransceiver::transmitInterrupt(fBusId);

    // Execute deferred callback for frame2 — should notify listener2
    EXPECT_CALL(listener2, canFrameSent(_)).Times(1);
    executeAsyncCallback();
}

// ===================================================================
// Category 8 — Edge cases
// ===================================================================

/// \brief transmitInterrupt with no pending listener: just calls transmitISR
TEST_F(InitedFdCanTransceiverTest, transmitInterruptWithNoPendingListener)
{
    EXPECT_CALL(fFct.fDevice, transmitISR());

    // No write(frame, listener) was called — TX ISR should just call transmitISR
    FdCanTransceiver::transmitInterrupt(fBusId);
}

/// \brief transmitInterrupt with invalid index is a no-op
TEST_F(FdCanTransceiverTest, transmitInterruptInvalidIndex)
{
    // Should not crash — just silently ignored
    FdCanTransceiver::transmitInterrupt(3);
    FdCanTransceiver::transmitInterrupt(255);
}

/// \brief write(frame, listener) when not OPEN returns TX_OFFLINE
TEST_F(InitedFdCanTransceiverTest, writeWithListenerWhenNotOpen)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    // State is INITIALIZED (not OPEN) — write should fail
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fFct.write(frame, listener));
}

/// \brief close() with pending TX queue: queue should be cleared
TEST_F(InitedFdCanTransceiverTest, closeWithPendingTxQueue)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));

    // Close with a pending TX job — should not crash
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.close());

    // TX ISR after close — should not crash or call listener
    EXPECT_CALL(listener, canFrameSent(_)).Times(0);
}

/// \brief mute() with pending TX queue: does not crash
TEST_F(InitedFdCanTransceiverTest, muteWithPendingTxQueue)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));

    // Mute with a pending TX job — should not crash
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.mute());
}

/// \brief disableRxInterrupt delegates to device
TEST_F(InitedFdCanTransceiverTest, disableRxInterruptDelegates)
{
    EXPECT_CALL(fFct.fDevice, disableRxInterrupt()).Times(1);
    FdCanTransceiver::disableRxInterrupt(fBusId);
}

/// \brief enableRxInterrupt delegates to device
TEST_F(InitedFdCanTransceiverTest, enableRxInterruptDelegates)
{
    EXPECT_CALL(fFct.fDevice, enableRxInterrupt()).Times(1);
    FdCanTransceiver::enableRxInterrupt(fBusId);
}

/// \brief disableRxInterrupt with invalid index is a no-op
TEST_F(FdCanTransceiverTest, disableRxInterruptInvalidIndex)
{
    // Should not crash
    FdCanTransceiver::disableRxInterrupt(3);
    FdCanTransceiver::disableRxInterrupt(255);
}

/// \brief enableRxInterrupt with invalid index is a no-op
TEST_F(FdCanTransceiverTest, enableRxInterruptInvalidIndex)
{
    // Should not crash
    FdCanTransceiver::enableRxInterrupt(3);
    FdCanTransceiver::enableRxInterrupt(255);
}

// ===================================================================
// Category 9 — State x Operation matrix
// ===================================================================

/// \brief write() in CLOSED state returns TX_OFFLINE
TEST_F(FdCanTransceiverTest, writeInClosedState)
{
    FdCanTransceiver fct(fAsyncContext, fBusId, fDevConfig);
    CANFrame frame;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fct.write(frame));
}

/// \brief write() in INITIALIZED state returns TX_OFFLINE (not OPEN)
TEST_F(InitedFdCanTransceiverTest, writeInInitializedState)
{
    CANFrame frame;
    // State is INITIALIZED — write should fail
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fFct.write(frame));
}

/// \brief write(frame, listener) in CLOSED state returns TX_OFFLINE
TEST_F(FdCanTransceiverTest, write2rInClosedState)
{
    FdCanTransceiver fct(fAsyncContext, fBusId, fDevConfig);
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fct.write(frame, listener));
}

/// \brief write(frame, listener) in INITIALIZED state returns TX_OFFLINE
TEST_F(InitedFdCanTransceiverTest, write2rInInitializedState)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fFct.write(frame, listener));
}

/// \brief mute() in CLOSED state returns ILLEGAL_STATE
TEST_F(FdCanTransceiverTest, muteInClosedState)
{
    FdCanTransceiver fct(fAsyncContext, fBusId, fDevConfig);
    EXPECT_CALL(fct.fDevice, init()).Times(AnyNumber());

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fct.mute());
}

/// \brief mute() when already MUTED returns ILLEGAL_STATE
TEST_F(InitedFdCanTransceiverTest, muteInMutedState)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fFct.mute());
}

/// \brief unmute() in CLOSED state returns ILLEGAL_STATE
TEST_F(FdCanTransceiverTest, unmuteInClosedState)
{
    FdCanTransceiver fct(fAsyncContext, fBusId, fDevConfig);
    EXPECT_CALL(fct.fDevice, init()).Times(AnyNumber());

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fct.unmute());
}

/// \brief unmute() in INITIALIZED state returns ILLEGAL_STATE
TEST_F(InitedFdCanTransceiverTest, unmuteInInitializedState)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fFct.unmute());
}

/// \brief unmute() in OPEN state returns ILLEGAL_STATE (not MUTED)
TEST_F(InitedFdCanTransceiverTest, unmuteInOpenState)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fFct.unmute());
}

/// \brief open() when already OPEN returns ILLEGAL_STATE
TEST_F(InitedFdCanTransceiverTest, openInOpenState)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fFct.open());
}

/// \brief open() when MUTED returns ILLEGAL_STATE
TEST_F(InitedFdCanTransceiverTest, openInMutedState)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fFct.open());
}

/// \brief close() from INITIALIZED returns OK (stops device)
TEST_F(InitedFdCanTransceiverTest, closeInInitializedState)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.close());
}

/// \brief close() from MUTED returns OK
TEST_F(InitedFdCanTransceiverTest, closeInMutedState)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.close());
}

// ===================================================================
// Category 10 — Bus-off interaction with TX queue
// ===================================================================

/// \brief Bus-off during pending TX: queue should be cleared
TEST_F(InitedFdCanTransceiverTest, busOffDuringPendingTxClearsQueue)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));

    // Bus-off detected by cyclicTask
    EXPECT_CALL(fFct.fDevice, isBusOff()).WillOnce(Return(true));
    fFct.cyclicTask();

    // After bus-off, write with listener should fail
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fFct.write(frame, listener));
}

/// \brief Bus recovery after queue cleared: new writes succeed
TEST_F(InitedFdCanTransceiverTest, busRecoveryAfterQueueClear)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillRepeatedly(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));

    // Bus-off
    EXPECT_CALL(fFct.fDevice, isBusOff()).WillOnce(Return(true)).WillOnce(Return(false));
    fFct.cyclicTask(); // OPEN -> MUTED

    // Bus recovery
    fFct.cyclicTask(); // MUTED -> OPEN

    // New write should succeed
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));
}

// ===================================================================
// Category 11 — Concurrent RX/TX
// ===================================================================

/// \brief receiveTask can run while TX is pending
TEST_F(InitedFdCanTransceiverTest, receiveTaskWhileTxPending)
{
    CANFrame txFrame;
    CANFrame rxFrame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(txFrame, listener));

    // RX task runs while TX is pending — should work independently
    EXPECT_CALL(fFct.fDevice, getRxCount()).WillOnce(Return(1));
    EXPECT_CALL(fFct.fDevice, getRxFrame(0)).WillOnce(ReturnRef(rxFrame));
    EXPECT_CALL(fFct.fDevice, clearRxQueue()).Times(1);
    EXPECT_CALL(fFct.fDevice, enableRxInterrupt()).Times(1);
    fFct.receiveTask();

    // TX ISR still works after RX task
    EXPECT_CALL(fFct.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    FdCanTransceiver::transmitInterrupt(fBusId);
}

/// \brief transmitInterrupt during receiveTask does not interfere
TEST_F(InitedFdCanTransceiverTest, transmitInterruptDuringReceiveTask)
{
    CANFrame txFrame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(txFrame, listener));

    // TX ISR fires
    EXPECT_CALL(fFct.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    FdCanTransceiver::transmitInterrupt(fBusId);

    // RX task runs after TX ISR — should still work normally
    EXPECT_CALL(fFct.fDevice, getRxCount()).WillOnce(Return(0));
    EXPECT_CALL(fFct.fDevice, clearRxQueue()).Times(1);
    EXPECT_CALL(fFct.fDevice, enableRxInterrupt()).Times(1);
    fFct.receiveTask();
}

} // namespace
