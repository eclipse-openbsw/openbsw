// Copyright 2024 Accenture.

#include "can/transceiver/canflex2/CanFlex2Transceiver.h"

#include "async/AsyncMock.h"
#include "bsp/timer/SystemTimerMock.h"
#include "can/CANFrameSentListenerMock.h"
#include "can/CanPhyMock.h"
#include "can/FilterMock.h"
#include "can/FilteredCANFrameSentListenerMock.h"
#include "power/PowerStateControllerMock.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::can;
using namespace ::testing;

class CanFlex2TransceiverTest : public Test
{
public:
    ::async::AsyncMock fAsyncMock;
    ::async::ContextType fAsyncContext = 0;
    uint8_t fBusId                     = 0xFF;
    ::bios::FlexCANDevice::Config fDevConfig;
    ::bios::CanPhyMock fCanPhy;
    ::power::PowerStateControllerMock fPowerStateController;
    FlexCANDevice* fpFlexCANDevice;

    void setDefaultFlexCANDeviceExpectations()
    {
        EXPECT_CALL(*fpFlexCANDevice, init())
            .WillRepeatedly(Return(ICanTransceiver::ErrorCode::CAN_ERR_OK));
        EXPECT_CALL(*fpFlexCANDevice, getFirstCanId()).WillRepeatedly(Return(0));
        EXPECT_CALL(*fpFlexCANDevice, getBaudrate()).WillRepeatedly(Return(921600));
        EXPECT_CALL(*fpFlexCANDevice, start())
            .WillRepeatedly(Return(ICanTransceiver::ErrorCode::CAN_ERR_OK));
        EXPECT_CALL(*fpFlexCANDevice, mute()).WillRepeatedly(Return());
        EXPECT_CALL(*fpFlexCANDevice, stop()).WillRepeatedly(Return());
        EXPECT_CALL(fAsyncMock, scheduleAtFixedRate(fAsyncContext, _, _, _, _)).Times(AnyNumber());
        EXPECT_CALL(fAsyncMock, execute(fAsyncContext, _))
            .Times(AnyNumber())
            .WillRepeatedly([](auto, auto& runnable) { runnable.execute(); });
    }
};

TEST_F(CanFlex2TransceiverTest, init)
{
    ::bios::CanFlex2Transceiver cft(
        fAsyncContext, fBusId, fDevConfig, fCanPhy, fPowerStateController);

    fpFlexCANDevice = fCanPhy.getFlexCANDevice();

    EXPECT_CALL(*fpFlexCANDevice, init())
        .WillRepeatedly(Return(ICanTransceiver::ErrorCode::CAN_ERR_OK));

    // normal case
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, cft.init());

    // subsequent init should fail
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, cft.init());
}

TEST_F(CanFlex2TransceiverTest, initFailure)
{
    ::bios::CanFlex2Transceiver cft(
        fAsyncContext, fBusId, fDevConfig, fCanPhy, fPowerStateController);

    fpFlexCANDevice = fCanPhy.getFlexCANDevice();

    // report failure when underlying hardware cannot init
    EXPECT_CALL(*fpFlexCANDevice, init())
        .WillRepeatedly(Return(ICanTransceiver::ErrorCode::CAN_ERR_INIT_FAILED));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_INIT_FAILED, cft.init());
}

class InitedCanFlex2TransceiverTest : public CanFlex2TransceiverTest
{
public:
    InitedCanFlex2TransceiverTest()
    : fCft(fAsyncContext, fBusId, fDevConfig, fCanPhy, fPowerStateController)
    {
        fpFlexCANDevice = fCanPhy.getFlexCANDevice();
        setDefaultFlexCANDeviceExpectations();
        EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.init());
    }

    ::bios::CanFlex2Transceiver fCft;
};

class InitedNoTimeoutCanFlex2TransceiverTest : public CanFlex2TransceiverTest
{
public:
    InitedNoTimeoutCanFlex2TransceiverTest()
    : fCft(fAsyncContext, fBusId, fDevConfig, fCanPhy, fPowerStateController)
    {
        fpFlexCANDevice = fCanPhy.getFlexCANDevice();
        setDefaultFlexCANDeviceExpectations();
        EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.init());
    }

    ::bios::CanFlex2Transceiver fCft;
};

TEST_F(InitedCanFlex2TransceiverTest, open)
{
    // INITIALIZED->OPEN
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.open());

    // repeated opens should fail
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fCft.open());
}

TEST_F(InitedNoTimeoutCanFlex2TransceiverTest, openNoTimeout)
{
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.open());
}

TEST_F(InitedCanFlex2TransceiverTest, close)
{
    // not opened yet, so should fail
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fCft.close());

    // regular open
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.close());

    // can also be open but muted
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.mute());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.close());

    // can't write while closed
    can::CANFrame frame;
    fCft.write(frame);
}

TEST_F(InitedNoTimeoutCanFlex2TransceiverTest, closeNoTimeout)
{
    // regular open
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.close());
}

TEST_F(InitedCanFlex2TransceiverTest, write1)
{
    ::can::CANFrame frame;

    EXPECT_CALL(*fpFlexCANDevice, getTransmitBuffer(_, _)).WillRepeatedly(Return(0));

    EXPECT_CALL(*fpFlexCANDevice, transmit(_, _, _))
        .WillOnce(Return(ICanTransceiver::ErrorCode::CAN_ERR_OK))
        .WillOnce(Return(ICanTransceiver::ErrorCode::CAN_ERR_OK))
        .WillOnce(Return(ICanTransceiver::ErrorCode::CAN_ERR_TX_FAIL));

    // test inited path
    fCft.write(frame);

    // test opened path
    fCft.open();
    fCft.write(frame);

    // test opened path with write failure
    fCft.write(frame);
}

TEST_F(InitedCanFlex2TransceiverTest, writeFullBuffer)
{
    ::can::CANFrame frame;

    EXPECT_CALL(*fpFlexCANDevice, getTransmitBuffer(_, _))
        .WillRepeatedly(Return(FlexCANDevice::TRANSMIT_BUFFER_UNAVAILABLE));

    fCft.write(frame);
}

TEST_F(InitedCanFlex2TransceiverTest, write2rOk)
{
    ::can::CANFrame frame;
    CANFrameSentListenerMock listener;

    EXPECT_CALL(*fpFlexCANDevice, getTransmitBuffer(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*fpFlexCANDevice, transmit(_, _, true))
        .WillOnce(Return(ICanTransceiver::ErrorCode::CAN_ERR_OK));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.write(frame, listener));
}

TEST_F(InitedCanFlex2TransceiverTest, write2rFail)
{
    ::can::CANFrame frame;
    CANFrameSentListenerMock listener;

    // do it again, but with transmit failure
    EXPECT_CALL(*fpFlexCANDevice, getTransmitBuffer(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*fpFlexCANDevice, transmit(_, _, _))
        .WillOnce(Return(ICanTransceiver::ErrorCode::CAN_ERR_TX_FAIL));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_FAIL, fCft.write(frame, listener));
}

TEST_F(InitedCanFlex2TransceiverTest, write2rFailsDueToMissingTransmitBuffer)
{
    ::can::CANFrame frame;
    CANFrameSentListenerMock listener;

    // do it again, but with transmit failure
    EXPECT_CALL(*fpFlexCANDevice, getTransmitBuffer(_, _))
        .WillOnce(Return(FlexCANDevice::TRANSMIT_BUFFER_UNAVAILABLE));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fCft.write(frame, listener));
}

TEST_F(InitedCanFlex2TransceiverTest, writeWhileMuted)
{
    can::CANFrame frame;

    fCft.open();
    fCft.mute();
    fCft.write(frame);
}

TEST_F(InitedCanFlex2TransceiverTest, writeOverFillQueue)
{
    can::CANFrame frame;
    CANFrameSentListenerMock listener;

    fCft.open();

    EXPECT_CALL(*fpFlexCANDevice, getTransmitBuffer(_, _)).WillRepeatedly(Return(0));

    EXPECT_CALL(*fpFlexCANDevice, transmit(_, _, _))
        .WillRepeatedly(Return(ICanTransceiver::ErrorCode::CAN_ERR_OK));

    // the items are popped from the queue in canFrameSentCallback, so we
    // never call it to simulate a 'queue full' situation

    // queue is length 3
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.write(frame, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL, fCft.write(frame, listener));
}

TEST_F(InitedCanFlex2TransceiverTest, muteNotOpen)
{
    // not open yet; can't be muted
    EXPECT_CALL(*fpFlexCANDevice, mute()).Times(0);
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fCft.mute());
}

TEST_F(InitedCanFlex2TransceiverTest, muteOpen)
{
    // open the device
    fCft.open();
    EXPECT_CALL(*fpFlexCANDevice, mute()).Times(1);
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.mute());

    // subsequent mutes should fail
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fCft.mute());
}

TEST_F(InitedCanFlex2TransceiverTest, unmute)
{
    fCft.open();

    // not muted, can't unmute
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_ILLEGAL_STATE, fCft.unmute());
    // mute it
    EXPECT_CALL(*fpFlexCANDevice, mute()).Times(1);
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.mute());
    // now we should be able to unmute
    EXPECT_CALL(*fpFlexCANDevice, unmute()).Times(1);
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.unmute());
}

TEST_F(InitedCanFlex2TransceiverTest, wokenUp)
{
    EXPECT_CALL(*fpFlexCANDevice, wokenUp()).WillOnce(Return(false)).WillOnce(Return(true));

    EXPECT_FALSE(fCft.wokenUp());
    EXPECT_TRUE(fCft.wokenUp());
}

TEST_F(InitedCanFlex2TransceiverTest, getBaudrate)
{
    EXPECT_CALL(*fpFlexCANDevice, getBaudrate()).WillOnce(Return(123456U));
    EXPECT_EQ(123456U, fCft.getBaudrate());
}

TEST_F(InitedCanFlex2TransceiverTest, getFirstFrameId)
{
    EXPECT_CALL(*fpFlexCANDevice, getFirstCanId()).WillOnce(Return(1234));
    EXPECT_EQ(1234, fCft.getFirstFrameId());
}

TEST_F(InitedCanFlex2TransceiverTest, resetFirstFrame)
{
    EXPECT_CALL(*fpFlexCANDevice, resetFirstFrame()).Times(1);
    fCft.resetFirstFrame();
}

TEST_F(InitedCanFlex2TransceiverTest, getRxAlive)
{
    EXPECT_CALL(*fpFlexCANDevice, getBusOffState()).WillRepeatedly(Return(FlexCANDevice::BUS_ON));
    EXPECT_CALL(fCanPhy, getPhyErrorStatus(_)).WillRepeatedly(Return(CanPhy::CAN_PHY_ERROR_NONE));

    // unmuted case
    EXPECT_FALSE(fCft.getRxAlive());

    // muted case
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.open());
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.mute());

    EXPECT_CALL(*fpFlexCANDevice, getFirstCanId())
        .WillOnce(Return(1234))  // set fRxAlive
        .WillOnce(Return(0))     // clear fRxAlive
        .WillOnce(Return(1234)); // set fRxAlive

    fCft.cyclicTask(); // set fRxAlive
    EXPECT_TRUE(fCft.getRxAlive());
    fCft.cyclicTask(); // clear fRxAlive
    EXPECT_FALSE(fCft.getRxAlive());
    // third call sets getFirstCanId
    EXPECT_TRUE(fCft.getRxAlive());
}

TEST_F(InitedCanFlex2TransceiverTest, expiredBusOff)
{
    EXPECT_CALL(*fpFlexCANDevice, clearRxAlive()).WillRepeatedly(Return());

    EXPECT_CALL(*fpFlexCANDevice, getBusOffState())
        .WillOnce(Return(FlexCANDevice::BUS_OFF))
        .WillOnce(Return(FlexCANDevice::BUS_ON))
        .WillOnce(Return(FlexCANDevice::BUS_OFF))
        .WillRepeatedly(Return(FlexCANDevice::BUS_ON));

    EXPECT_CALL(fCanPhy, getPhyErrorStatus(_))
        .WillOnce(Return(CanPhy::CAN_PHY_ERROR_NONE))
        .WillOnce(Return(CanPhy::CAN_PHY_ERROR_NONE))
        .WillOnce(Return(CanPhy::CAN_PHY_ERROR_NONE))
        .WillOnce(Return(CanPhy::CAN_PHY_ERROR_NONE))
        .WillOnce(Return(CanPhy::CAN_PHY_ERROR))
        .WillOnce(Return(CanPhy::CAN_PHY_ERROR))
        .WillOnce(Return(CanPhy::CAN_PHY_ERROR_UNSUPPORTED));

    EXPECT_CALL(*fpFlexCANDevice, getRxAlive()).WillOnce(Return(0)).WillRepeatedly(Return(1));

    fCft.cyclicTask(); // update fRxAlive
    fCft.getRxAlive();

    // test the fTransceiverState BUS_OFF/other branches
    // fTransceiverState is now BUS_OFF; getBusOffState will return BUS_ON
    fCft.cyclicTask();
    fCft.getRxAlive();

    // fTransceiverState is now ACTIVE; getBusOffState will return BUS_OFF
    fCft.cyclicTask();

    // test phyState CAN_PHY_ERROR_UNSUPPORTED/not branches
    // CAN_PHY_ERROR_NONE
    fCft.cyclicTask();
    // fIsPhyErrorPresent is now false

    // CAN_PHY_ERROR
    fCft.cyclicTask();
    // fIsPhyErrorPresent is now true

    // CAN_PHY_ERROR
    fCft.cyclicTask();

    // CAN_PHY_ERROR_UNSUPPORTED
    fCft.cyclicTask();
}

TEST_F(InitedCanFlex2TransceiverTest, expiredBusOn)
{
    EXPECT_CALL(*fpFlexCANDevice, getRxAlive());
    EXPECT_CALL(*fpFlexCANDevice, clearRxAlive());
    EXPECT_CALL(*fpFlexCANDevice, getBusOffState()).WillOnce(Return(FlexCANDevice::BUS_ON));

    EXPECT_CALL(fCanPhy, getPhyErrorStatus(_)).WillRepeatedly(Return(CanPhy::CAN_PHY_ERROR_NONE));

    fCft.cyclicTask(); // update fRxAlive

    // test the fTransceiverState ACTIVE/other branches
    // todo
}

TEST_F(InitedCanFlex2TransceiverTest, receiveTask1)
{
    // TODO: doesn't really test anything of value
    EXPECT_CALL(*fpFlexCANDevice, isRxQueueEmpty()).WillOnce(Return(true));
    fCft.receiveTask();
}

TEST_F(InitedCanFlex2TransceiverTest, receiveTask2)
{
    can::CANFrame frame;

    fCft.open();

    // TODO: doesn't really test anything of value
    EXPECT_CALL(*fpFlexCANDevice, isRxQueueEmpty()).WillOnce(Return(false)).WillOnce(Return(true));

    EXPECT_CALL(*fpFlexCANDevice, getRxFrameQueueFront()).WillOnce(ReturnRef(frame));

    EXPECT_CALL(*fpFlexCANDevice, dequeueRxFrame()).WillOnce(ReturnRef(frame));

    EXPECT_CALL(*fpFlexCANDevice, transmit(_, _, _))
        .WillRepeatedly(Return(ICanTransceiver::ErrorCode::CAN_ERR_OK));

    fCft.receiveTask();
}

TEST_F(InitedCanFlex2TransceiverTest, getAllCanFlex2Transceivers)
{
    CanFlex2Transceiver** transceivers = fCft.getAllCanFlex2Transceivers();
    EXPECT_EQ(&fCft, transceivers[0]);
}

TEST_F(InitedCanFlex2TransceiverTest, receiveInterrupt)
{
    EXPECT_CALL(*fpFlexCANDevice, receiveISR(_)).WillOnce(Return(0));
    CanFlex2Transceiver::receiveInterrupt(0);
}

TEST_F(InitedCanFlex2TransceiverTest, transmitInterrupt)
{
    EXPECT_CALL(*fpFlexCANDevice, transmitISR());
    CanFlex2Transceiver::transmitInterrupt(0);
}

TEST_F(InitedCanFlex2TransceiverTest, canFrameSentCallbackWithOneFrame)
{
    can::CANFrame frame;

    EXPECT_CALL(*fpFlexCANDevice, getTransmitBuffer(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*fpFlexCANDevice, transmit(_, _, _))
        .WillRepeatedly(Return(ICanTransceiver::ErrorCode::CAN_ERR_OK));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.write(frame));

    fCft.canFrameSentCallback();
}

TEST_F(InitedCanFlex2TransceiverTest, canFrameSentCallbackWithTwoFramesInStateInitialized)
{
    CANFrameSentListenerMock listener;
    can::CANFrame frame1;
    can::CANFrame frame2;

    EXPECT_CALL(*fpFlexCANDevice, getTransmitBuffer(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*fpFlexCANDevice, transmit(_, _, _))
        .WillRepeatedly(Return(ICanTransceiver::ErrorCode::CAN_ERR_OK));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.write(frame1, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.write(frame2, listener));

    fCft.canFrameSentCallback();
}

TEST_F(InitedCanFlex2TransceiverTest, canFrameSentCallbackWithTwoFramesInStateOpen)
{
    CANFrameSentListenerMock listener;
    can::CANFrame frame1;
    can::CANFrame frame2;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.open());

    EXPECT_CALL(*fpFlexCANDevice, getTransmitBuffer(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*fpFlexCANDevice, transmit(_, _, _))
        .WillRepeatedly(Return(ICanTransceiver::ErrorCode::CAN_ERR_OK));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.write(frame1, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.write(frame2, listener));

    fCft.canFrameSentCallback();
}

TEST_F(InitedCanFlex2TransceiverTest, canFrameSentCallbackWithTwoFramesIsAbortedWhenMuted)
{
    CANFrameSentListenerMock listener;
    can::CANFrame frame1;
    can::CANFrame frame2;

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.open());

    EXPECT_CALL(*fpFlexCANDevice, getTransmitBuffer(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*fpFlexCANDevice, transmit(_, _, _))
        .WillOnce(Return(ICanTransceiver::ErrorCode::CAN_ERR_OK));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.write(frame1, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.write(frame2, listener));

    fCft.mute();

    fCft.canFrameSentCallback();
}

TEST_F(InitedCanFlex2TransceiverTest, canFrameSentCallbackWithTwoFramesFailure)
{
    CANFrameSentListenerMock listener;
    can::CANFrame frame1;
    can::CANFrame frame2;

    EXPECT_CALL(*fpFlexCANDevice, getTransmitBuffer(_, _)).WillRepeatedly(Return(0));

    EXPECT_CALL(*fpFlexCANDevice, transmit(_, _, _))
        .WillOnce(Return(ICanTransceiver::ErrorCode::CAN_ERR_TX_FAIL))
        .WillOnce(Return(ICanTransceiver::ErrorCode::CAN_ERR_OK));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_TX_FAIL, fCft.write(frame1, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.write(frame2, listener));

    fCft.canFrameSentCallback();
}

TEST_F(InitedCanFlex2TransceiverTest, canFrameSentCallbackWithTwoFramesIsAbortedWhenNextSendFails)
{
    CANFrameSentListenerMock listener;
    can::CANFrame frame1;
    can::CANFrame frame2;

    EXPECT_CALL(*fpFlexCANDevice, getTransmitBuffer(_, _)).WillRepeatedly(Return(0));

    EXPECT_CALL(*fpFlexCANDevice, transmit(_, _, _))
        .WillOnce(Return(ICanTransceiver::ErrorCode::CAN_ERR_OK))
        .WillOnce(Return(ICanTransceiver::ErrorCode::CAN_ERR_TX_FAIL));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.write(frame1, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.write(frame2, listener));

    fCft.canFrameSentCallback();
}

TEST_F(
    InitedCanFlex2TransceiverTest,
    canFrameSentCallbackWithTwoFramesIsAbortedWhenNoTransmitBufferAvailable)
{
    CANFrameSentListenerMock listener;
    can::CANFrame frame1;
    can::CANFrame frame2;

    EXPECT_CALL(*fpFlexCANDevice, getTransmitBuffer(_, _))
        .WillOnce(Return(0))
        .WillOnce(Return(FlexCANDevice::TRANSMIT_BUFFER_UNAVAILABLE));

    EXPECT_CALL(*fpFlexCANDevice, transmit(_, _, _))
        .WillOnce(Return(ICanTransceiver::ErrorCode::CAN_ERR_OK));

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.write(frame1, listener));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.write(frame2, listener));

    fCft.canFrameSentCallback();
}

TEST_F(InitedCanFlex2TransceiverTest, notifyRegisteredSentListenerMatch)
{
    can::CANFrame frame;
    FilteredCANFrameSentListenerMock listener;
    FilterMock filter;

    EXPECT_CALL(*fpFlexCANDevice, transmit(_, _, _))
        .WillRepeatedly(Return(ICanTransceiver::ErrorCode::CAN_ERR_OK));

    EXPECT_CALL(*fpFlexCANDevice, getTransmitBuffer(_, _)).WillOnce(Return(0));

    EXPECT_CALL(listener, canFrameSent(_)).Times(1);

    ::testing::SystemTimerMock systemTimer;
    EXPECT_CALL(systemTimer, getSystemTimeUs32Bit()).WillOnce(Return(10U));

    fCft.addCANFrameSentListener(listener);

    // will call notifyRegisteredSentListener
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fCft.write(frame));

    fCft.removeCANFrameSentListener(listener);
}

} // namespace
