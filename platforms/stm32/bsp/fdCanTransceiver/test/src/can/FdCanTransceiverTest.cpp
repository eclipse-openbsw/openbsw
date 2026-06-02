/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "can/transceiver/fdcan/FdCanTransceiver.h"

#include "async/AsyncMock.h"
#include "can/canframes/ICANFrameSentListener.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::can;
using namespace ::testing;
using namespace ::bios;

// Mock for ICANFrameSentListener (write-with-listener / DoCAN callback)
class MockCANFrameSentListener : public ::can::ICANFrameSentListener
{
public:
    MOCK_METHOD(void, canFrameSent, (::can::CANFrame const&), (override));
};

// Fixture: bare transceiver (CLOSED state)
class FdCanTransceiverTest : public Test
{
public:
    FdCanTransceiverTest() { fDevConfig.baudrate = 500000U; }

    ::async::AsyncMock fAsyncMock;
    ::async::ContextType fAsyncContext = 0;
    uint8_t fBusId                     = 0;
    FdCanDevice::Config fDevConfig{};
};

// Fixture: transceiver in INITIALIZED state (auto-executes async)
class InitedFdCanTransceiverTest : public FdCanTransceiverTest
{
public:
    InitedFdCanTransceiverTest() : fFct(fAsyncContext, fBusId, fDevConfig)
    {
        fFct.fDevice.setupDefaultTransmitISR();
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

// Fixture: transceiver in INITIALIZED state with manual async control
// (async::execute is captured but NOT auto-executed)
class ManualAsyncFdCanTransceiverTest : public FdCanTransceiverTest
{
public:
    ManualAsyncFdCanTransceiverTest() : fFct(fAsyncContext, fBusId, fDevConfig)
    {
        fFct.fDevice.setupDefaultTransmitISR();
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

TEST_F(FdCanTransceiverTest, initFromClosed)
{
    FdCanTransceiver fct(fAsyncContext, fBusId, fDevConfig);

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fct.init());
}

TEST_F(FdCanTransceiverTest, initTwiceFails)
{
    FdCanTransceiver fct(fAsyncContext, fBusId, fDevConfig);

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fct.init());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fct.init());
}

TEST_F(InitedFdCanTransceiverTest, open)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
}

TEST_F(InitedFdCanTransceiverTest, openTwiceFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fFct.open());
}

TEST_F(InitedFdCanTransceiverTest, openFromClosed)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.close());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
}

TEST_F(InitedFdCanTransceiverTest, closeFromOpen)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.close());
}

TEST_F(InitedFdCanTransceiverTest, closeFromClosedReturnsOk)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.close());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.close());
}

TEST_F(InitedFdCanTransceiverTest, muteFromOpen)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.mute());
}

TEST_F(InitedFdCanTransceiverTest, muteFromInitializedFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fFct.mute());
}

TEST_F(InitedFdCanTransceiverTest, unmuteFromMuted)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.unmute());
}

TEST_F(InitedFdCanTransceiverTest, unmuteFromOpenFails)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fFct.unmute());
}

TEST_F(InitedFdCanTransceiverTest, writeWhenOpen)
{
    CANFrame frame;

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame));
}

TEST_F(InitedFdCanTransceiverTest, writeWhenMutedFails)
{
    CANFrame frame;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fFct.write(frame));
}

TEST_F(InitedFdCanTransceiverTest, writeWhenFifoFull)
{
    CANFrame frame;

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(false));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fFct.write(frame));
}

TEST_F(InitedFdCanTransceiverTest, getBaudrateComesFromDeviceConfig)
{
    EXPECT_EQ(500000U, fFct.getBaudrate());
}

TEST_F(InitedFdCanTransceiverTest, getHwQueueTimeoutComputedFromBaudrate)
{
    // 3 TX FIFO slots x 117 bit = 351000 bit-ms/s, rounded up to a full ms:
    // 500 kbit/s -> ceil(0.702 ms) = 1 ms (never 0)
    EXPECT_EQ(1U, fFct.getHwQueueTimeout());

    // 125 kbit/s -> ceil(351000 / 125000) = ceil(2.808) = 3 ms
    fDevConfig.baudrate = 125000U;
    FdCanTransceiver slowFct(fAsyncContext, fBusId, fDevConfig);
    EXPECT_EQ(3U, slowFct.getHwQueueTimeout());
}

TEST_F(InitedFdCanTransceiverTest, receiveTask)
{
    CANFrame frame;

    EXPECT_CALL(fFct.fDevice, getRxCount()).WillOnce(Return(0));
    EXPECT_CALL(fFct.fDevice, clearRxQueue()).Times(1);
    EXPECT_CALL(fFct.fDevice, enableRxInterrupt()).Times(1);

    fFct.receiveTask();
}

TEST_F(InitedFdCanTransceiverTest, cyclicTaskBusOff)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, isBusOff()).WillOnce(Return(true));
    fFct.cyclicTask();

    // After bus-off, write should fail
    CANFrame frame;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fFct.write(frame));
}

TEST_F(InitedFdCanTransceiverTest, cyclicTaskBusRecovery)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, isBusOff()).WillOnce(Return(true)).WillOnce(Return(false));

    fFct.cyclicTask(); // bus-off -> MUTED
    fFct.cyclicTask(); // bus-on -> OPEN

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(true));
    CANFrame frame;
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame));
}

TEST_F(InitedFdCanTransceiverTest, shutdown)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    fFct.shutdown();
}

TEST_F(InitedFdCanTransceiverTest, receiveInterrupt)
{
    EXPECT_CALL(fFct.fDevice, receiveISR(_)).WillOnce(Return(0));
    FdCanTransceiver::receiveInterrupt(fBusId);
}

TEST_F(InitedFdCanTransceiverTest, transmitInterrupt)
{
    EXPECT_CALL(fFct.fDevice, transmitISR());
    FdCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(FdCanTransceiverTest, receiveInterruptInvalidIndex)
{
    EXPECT_EQ(0U, FdCanTransceiver::receiveInterrupt(3));
}

TEST_F(InitedFdCanTransceiverTest, write2rOk)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));
}

TEST_F(InitedFdCanTransceiverTest, write2rFail)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(false));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fFct.write(frame, listener));
}

TEST_F(InitedFdCanTransceiverTest, write2rWhenMuted)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fFct.write(frame, listener));
}

TEST_F(InitedFdCanTransceiverTest, write2rDoesNotCallListenerSynchronously)
{
    CANFrame frame;
    StrictMock<MockCANFrameSentListener> listener;

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(true));
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

TEST_F(InitedFdCanTransceiverTest, writeOverFillQueue)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillRepeatedly(Return(true));

    // Queue capacity is 3; items are popped only in canFrameSentAsyncCallback,
    // which we never trigger here to simulate queue-full.
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fFct.write(frame, listener));
}

TEST_F(ManualAsyncFdCanTransceiverTest, writeSecondFrameNotTransmittedImmediately)
{
    CANFrame frame1;
    CANFrame frame2;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    // Only ONE transmit call expected from the first write()
    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame1, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame2, listener));

    // At this point, only frame1 was transmitted to HW; frame2 waits in queue.
    Mock::VerifyAndClearExpectations(&fFct.fDevice);

    // Now simulate TX ISR for frame1 - this should transmit frame2
    EXPECT_CALL(fFct.fDevice, transmitISR());
    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(true));
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);

    FdCanTransceiver::transmitInterrupt(fBusId);
    // Execute the deferred async callback
    executeAsyncCallback();
}

TEST_F(InitedFdCanTransceiverTest, canFrameSentCallbackWithOneFrame)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));

    // TX ISR fires -> canFrameSentCallback -> async -> canFrameSentAsyncCallback
    EXPECT_CALL(fFct.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    FdCanTransceiver::transmitInterrupt(fBusId);
}

/// Note: S32K allows write in INITIALIZED, our port requires OPEN. Test uses OPEN.
TEST_F(InitedFdCanTransceiverTest, canFrameSentCallbackWithTwoFramesChained)
{
    MockCANFrameSentListener listener;
    CANFrame frame1;
    CANFrame frame2;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillRepeatedly(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame1, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame2, listener));

    // TX ISR for frame1 - pops frame1, notifies listener, transmits frame2
    EXPECT_CALL(fFct.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    FdCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedFdCanTransceiverTest, canFrameSentCallbackWithTwoFramesInStateOpen)
{
    MockCANFrameSentListener listener;
    CANFrame frame1;
    CANFrame frame2;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillRepeatedly(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame1, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame2, listener));

    EXPECT_CALL(fFct.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    FdCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedFdCanTransceiverTest, canFrameSentCallbackWithTwoFramesIsAbortedWhenMuted)
{
    MockCANFrameSentListener listener;
    CANFrame frame1;
    CANFrame frame2;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame1, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame2, listener));

    fFct.mute();

    // TX ISR fires - first frame was already submitted. Callback should pop
    // frame1 and notify listener, but NOT submit frame2 (muted).
    EXPECT_CALL(fFct.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    FdCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedFdCanTransceiverTest, canFrameSentCallbackWithTwoFramesFailure)
{
    MockCANFrameSentListener listener;
    CANFrame frame1;
    CANFrame frame2;

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(false)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    // First write fails at HW level
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fFct.write(frame1, listener));
    // Second write succeeds
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame2, listener));

    EXPECT_CALL(fFct.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    FdCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(InitedFdCanTransceiverTest, canFrameSentCallbackWithTwoFramesIsAbortedWhenNextSendFails)
{
    MockCANFrameSentListener listener;
    CANFrame frame1;
    CANFrame frame2;

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(true)).WillOnce(Return(false));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame1, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame2, listener));

    // TX ISR for frame1 - callback pops frame1, tries to send frame2 but
    // transmit returns false => aborted, queue cleared
    EXPECT_CALL(fFct.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    FdCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(ManualAsyncFdCanTransceiverTest, canFrameSentCallbackUsesAsyncExecute)
{
    CANFrame frame;
    StrictMock<MockCANFrameSentListener> listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));

    // TX ISR fires - should call async::execute, NOT call listener directly
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

TEST_F(ManualAsyncFdCanTransceiverTest, canFrameSentCallbackRunsInTaskContext)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(true));
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

TEST_F(InitedFdCanTransceiverTest, receiveInterruptPassesFilterToDevice)
{
    EXPECT_CALL(fFct.fDevice, receiveISR(_)).WillOnce(Return(1));
    FdCanTransceiver::receiveInterrupt(fBusId);
}

TEST_F(InitedFdCanTransceiverTest, receiveInterruptAcceptsAllFrames)
{
    EXPECT_CALL(fFct.fDevice, receiveISR(_)).WillOnce(Return(1)).WillOnce(Return(3));

    EXPECT_EQ(1U, FdCanTransceiver::receiveInterrupt(fBusId));
    EXPECT_EQ(3U, FdCanTransceiver::receiveInterrupt(fBusId));
}

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

TEST_F(InitedFdCanTransceiverTest, receiveTaskReEnablesInterrupt)
{
    EXPECT_CALL(fFct.fDevice, getRxCount()).WillOnce(Return(0));
    EXPECT_CALL(fFct.fDevice, clearRxQueue()).Times(1);
    EXPECT_CALL(fFct.fDevice, enableRxInterrupt()).Times(1);

    fFct.receiveTask();
}

/// Note: upstream AbstractCANTransceiver only calls SystemTimer when
/// IFilteredCANFrameSentListener is registered. This test verifies the
/// write succeeds and the transceiver doesn't crash without registered listeners.
TEST_F(InitedFdCanTransceiverTest, notifyRegisteredSentListenerMatch)
{
    CANFrame frame;

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame));
}

TEST_F(InitedFdCanTransceiverTest, notifyRegisteredSentListenerFromCallback)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(true));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));

    // TX ISR -> async callback -> pops queue -> calls listener.canFrameSent
    // Also calls notifyRegisteredSentListener (notifySentListeners)
    EXPECT_CALL(fFct.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    FdCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(
    ManualAsyncFdCanTransceiverTest, writeWithListenerThenTransmitInterruptTriggersDeferredCallback)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));

    // Step 1: TX ISR fires - defers to task context
    fCapturedRunnable = nullptr;
    EXPECT_CALL(fFct.fDevice, transmitISR());
    FdCanTransceiver::transmitInterrupt(fBusId);

    ASSERT_NE(nullptr, fCapturedRunnable);

    // Step 2: Task context executes - listener gets called
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    executeAsyncCallback();
}

TEST_F(ManualAsyncFdCanTransceiverTest, multipleWriteWithListenerProcessedSerially)
{
    CANFrame frame1;
    CANFrame frame2;
    MockCANFrameSentListener listener1;
    MockCANFrameSentListener listener2;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_, _))
        .WillOnce(Return(true))  // frame1
        .WillOnce(Return(true)); // frame2 (from callback chain)

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame1, listener1));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame2, listener2));

    // TX ISR for frame1
    fCapturedRunnable = nullptr;
    EXPECT_CALL(fFct.fDevice, transmitISR());
    FdCanTransceiver::transmitInterrupt(fBusId);

    // Execute deferred callback for frame1 - should notify listener1, submit frame2
    EXPECT_CALL(listener1, canFrameSent(_)).Times(1);
    EXPECT_CALL(listener2, canFrameSent(_)).Times(0);
    executeAsyncCallback();

    Mock::VerifyAndClearExpectations(&listener1);
    Mock::VerifyAndClearExpectations(&listener2);

    // TX ISR for frame2
    fCapturedRunnable = nullptr;
    EXPECT_CALL(fFct.fDevice, transmitISR());
    FdCanTransceiver::transmitInterrupt(fBusId);

    // Execute deferred callback for frame2 - should notify listener2
    EXPECT_CALL(listener2, canFrameSent(_)).Times(1);
    executeAsyncCallback();
}

TEST_F(InitedFdCanTransceiverTest, transmitInterruptWithNoPendingListener)
{
    EXPECT_CALL(fFct.fDevice, transmitISR());

    // No write(frame, listener) was called - TX ISR should just call transmitISR
    FdCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(FdCanTransceiverTest, transmitInterruptInvalidIndex)
{
    // Should not crash - just silently ignored
    FdCanTransceiver::transmitInterrupt(3);
    FdCanTransceiver::transmitInterrupt(255);
}

TEST_F(InitedFdCanTransceiverTest, writeWithListenerWhenNotOpen)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    // State is INITIALIZED (not OPEN) - write should fail
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fFct.write(frame, listener));
}

TEST_F(InitedFdCanTransceiverTest, closeWithPendingTxQueue)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));

    // Close with a pending TX job - should not crash
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.close());

    // TX ISR after close - should not crash or call listener
    EXPECT_CALL(listener, canFrameSent(_)).Times(0);
}

TEST_F(InitedFdCanTransceiverTest, muteWithPendingTxQueue)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));

    // Mute with a pending TX job - should not crash
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.mute());
}

TEST_F(InitedFdCanTransceiverTest, disableRxInterruptDelegates)
{
    EXPECT_CALL(fFct.fDevice, disableRxInterrupt()).Times(1);
    FdCanTransceiver::disableRxInterrupt(fBusId);
}

TEST_F(InitedFdCanTransceiverTest, enableRxInterruptDelegates)
{
    EXPECT_CALL(fFct.fDevice, enableRxInterrupt()).Times(1);
    FdCanTransceiver::enableRxInterrupt(fBusId);
}

TEST_F(FdCanTransceiverTest, disableRxInterruptInvalidIndex)
{
    // Should not crash
    FdCanTransceiver::disableRxInterrupt(3);
    FdCanTransceiver::disableRxInterrupt(255);
}

TEST_F(FdCanTransceiverTest, enableRxInterruptInvalidIndex)
{
    // Should not crash
    FdCanTransceiver::enableRxInterrupt(3);
    FdCanTransceiver::enableRxInterrupt(255);
}

TEST_F(FdCanTransceiverTest, writeInClosedState)
{
    FdCanTransceiver fct(fAsyncContext, fBusId, fDevConfig);
    CANFrame frame;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fct.write(frame));
}

TEST_F(InitedFdCanTransceiverTest, writeInInitializedState)
{
    CANFrame frame;
    // State is INITIALIZED - write should fail
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fFct.write(frame));
}

TEST_F(FdCanTransceiverTest, write2rInClosedState)
{
    FdCanTransceiver fct(fAsyncContext, fBusId, fDevConfig);
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fct.write(frame, listener));
}

TEST_F(InitedFdCanTransceiverTest, write2rInInitializedState)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fFct.write(frame, listener));
}

TEST_F(FdCanTransceiverTest, muteInClosedState)
{
    FdCanTransceiver fct(fAsyncContext, fBusId, fDevConfig);
    EXPECT_CALL(fct.fDevice, init()).Times(AnyNumber());

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fct.mute());
}

TEST_F(InitedFdCanTransceiverTest, muteInMutedState)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fFct.mute());
}

TEST_F(FdCanTransceiverTest, unmuteInClosedState)
{
    FdCanTransceiver fct(fAsyncContext, fBusId, fDevConfig);
    EXPECT_CALL(fct.fDevice, init()).Times(AnyNumber());

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fct.unmute());
}

TEST_F(InitedFdCanTransceiverTest, unmuteInInitializedState)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fFct.unmute());
}

TEST_F(InitedFdCanTransceiverTest, unmuteInOpenState)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fFct.unmute());
}

TEST_F(InitedFdCanTransceiverTest, openInOpenState)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fFct.open());
}

TEST_F(InitedFdCanTransceiverTest, openInMutedState)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fFct.open());
}

TEST_F(InitedFdCanTransceiverTest, closeInInitializedState)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.close());
}

TEST_F(InitedFdCanTransceiverTest, closeInMutedState)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.close());
}

TEST_F(InitedFdCanTransceiverTest, busOffDuringPendingTxClearsQueue)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));

    // Bus-off detected by cyclicTask
    EXPECT_CALL(fFct.fDevice, isBusOff()).WillOnce(Return(true));
    fFct.cyclicTask();

    // After bus-off, write with listener should fail
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_OFFLINE, fFct.write(frame, listener));
}

TEST_F(InitedFdCanTransceiverTest, busRecoveryAfterQueueClear)
{
    CANFrame frame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillRepeatedly(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));

    // Bus-off
    EXPECT_CALL(fFct.fDevice, isBusOff()).WillOnce(Return(true)).WillOnce(Return(false));
    fFct.cyclicTask(); // OPEN -> MUTED

    // Bus recovery
    fFct.cyclicTask(); // MUTED -> OPEN

    // New write should succeed
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(frame, listener));
}

TEST_F(InitedFdCanTransceiverTest, receiveTaskWhileTxPending)
{
    CANFrame txFrame;
    CANFrame rxFrame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(txFrame, listener));

    // RX task runs while TX is pending - should work independently
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

TEST_F(InitedFdCanTransceiverTest, transmitInterruptDuringReceiveTask)
{
    CANFrame txFrame;
    MockCANFrameSentListener listener;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.open());

    EXPECT_CALL(fFct.fDevice, transmit(_, _)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fFct.write(txFrame, listener));

    // TX ISR fires
    EXPECT_CALL(fFct.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    FdCanTransceiver::transmitInterrupt(fBusId);

    // RX task runs after TX ISR - should still work normally
    EXPECT_CALL(fFct.fDevice, getRxCount()).WillOnce(Return(0));
    EXPECT_CALL(fFct.fDevice, clearRxQueue()).Times(1);
    EXPECT_CALL(fFct.fDevice, enableRxInterrupt()).Times(1);
    fFct.receiveTask();
}

} // namespace
