// Copyright 2024 Accenture.

#include "can/canframes/BufferedCANFrame.h"
#include "can/canframes/CANFrameSentListenerMock.h"
#include "can/filter/FilterMock.h"
#include "can/framemgmt/AbstractBitFieldFilteredCANFrameListener.h"
#include "can/framemgmt/AbstractIntervalFilteredCANFrameListener.h"
#include "can/framemgmt/CANFrameListenerMock.h"
#include "can/framemgmt/FilteredCANFrameSentListenerMock.h"
#include "can/transceiver/AbstractCANTransceiverMock.h"
#include "can/transceiver/ICANTransceiverStateListener.h"

#include <bsp/timer/SystemTimerMock.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::can;
using namespace ::testing;

class receiverTestBase
{
public:
    void addExpectedCANFrame(CANFrame const& frame) { expectedFrame = frame; }

    void verify() { ASSERT_EQ(receivedFrame, expectedFrame); }

protected:
    CANFrame expectedFrame;
    CANFrame receivedFrame;
};

class tBitFieldListener
: public AbstractBitFieldFilteredCANFrameListener
, public receiverTestBase
{
public:
    tBitFieldListener()
    {
        ON_CALL(*this, frameReceived(_))
            .WillByDefault(Invoke(this, &tBitFieldListener::frameReceivedImplementation));
        ;
    }

    MOCK_METHOD1(frameReceived, void(CANFrame const& frame));

private:
    void frameReceivedImplementation(CANFrame const& frame) { receivedFrame = frame; }
};

class tIntervalListener
: public AbstractIntervalFilteredCANFrameListener
, public receiverTestBase
{
public:
    tIntervalListener()
    {
        ON_CALL(*this, frameReceived(_))
            .WillByDefault(Invoke(this, &tIntervalListener::frameReceivedImplementation));
        ;
    }

    MOCK_METHOD1(frameReceived, void(CANFrame const& frame));

    void verify() { ASSERT_EQ(expectedFrame, receivedFrame); }

private:
    void frameReceivedImplementation(CANFrame const& frame) { receivedFrame = frame; }
};

class aBitFieldFilter : public AbstractStaticBitFieldFilter
{
public:
    aBitFieldFilter() : fMask(0xFF) {}

    uint8_t getMaskValue(uint16_t /* byteIndex */) const override { return fMask; }

    uint8_t fMask;
};

class tAbstractBitFieldFilterListener
: public ICANFrameListener
, public receiverTestBase
{
public:
    tAbstractBitFieldFilterListener()
    {
        ON_CALL(*this, frameReceived(_))
            .WillByDefault(
                Invoke(this, &tAbstractBitFieldFilterListener::frameReceivedImplementation));
        filter.open();
    }

    MOCK_METHOD1(frameReceived, void(CANFrame const& frame));

    void verify() { ASSERT_EQ(expectedFrame, receivedFrame); }

    /**
     * @see    ICANFrameListener::getFilter()
     */
    IFilter& getFilter() override { return filter; }

    bool compareFilter(uint32_t value, uint8_t mask)
    {
        aBitFieldFilter lFilter;
        lFilter.add(value);
        lFilter.clear();
        lFilter.fMask = mask;
        return (filter == lFilter);
    }

private:
    void frameReceivedImplementation(CANFrame const& frame) { receivedFrame = frame; }

    aBitFieldFilter filter;
};

class tStateChangeListener
: public ICANTransceiverStateListener
, public ICANFrameSentListener
{
public:
    MOCK_METHOD2(
        canTransceiverStateChanged, void(ICanTransceiver& transceiver, CANTransceiverState state));

    MOCK_METHOD1(phyErrorOccurred, void(ICanTransceiver& transceiver));
    MOCK_METHOD1(canFrameSent, void(CANFrame const& frame));
};

class tSendFrameListener : public IFilteredCANFrameSentListener
{
public:
    MOCK_METHOD1(canFrameSent, void(CANFrame const& frame));

    IFilter& getFilter() override { return filter; }

private:
    aBitFieldFilter filter;
};

class tCanTransceiver : public AbstractCANTransceiverMock
{
public:
    tCanTransceiver() : AbstractCANTransceiverMock(0) {}

    using AbstractCANTransceiver::notifySentListeners;
};

class AbstractCANTransceiverTest : public ::testing::Test
{
public:
    AbstractCANTransceiverTest() { fpTransceiver = new AbstractCANTransceiverMock((0)); }

    ~AbstractCANTransceiverTest() override { delete fpTransceiver; }

protected:
    AbstractCANTransceiverMock* fpTransceiver;
    ::testing::SystemTimerMock fSystemTimer;
};

/**
 * @test
 * verification of constructor behaviour
 */
TEST_F(AbstractCANTransceiverTest, testConstructor)
{
    StrictMock<AbstractCANTransceiverMock> transceiver((0));
    ASSERT_EQ(ICanTransceiver::State::CLOSED, transceiver.getState());
}

/**
 * @test
 * verification of addCANFrameListener method of AbstractCANTransceiver
 */
TEST_F(AbstractCANTransceiverTest, testAddCANFrameListener)
{
    StrictMock<CANFrameListenerMock> listener;
    StrictMock<FilterMock> filter;
    EXPECT_CALL(listener, getFilter()).Times(1).WillOnce(ReturnRef(filter));
    EXPECT_CALL(filter, acceptMerger(_)).Times(1);

    // add listener the first time
    fpTransceiver->addCANFrameListener(listener);
    // add listener again
    fpTransceiver->addCANFrameListener(listener);
    // the same for an IntervalListener
    tIntervalListener listener2;
    // add VIP listener
    fpTransceiver->addVIPCANFrameListener(listener2);
    // add VIP listener again
    fpTransceiver->addVIPCANFrameListener(listener2);

    fpTransceiver->removeCANFrameListener(listener);
    fpTransceiver->removeCANFrameListener(listener2);
}

/**
 * @test
 * verification of removeCANFrameListener method
 */
TEST_F(AbstractCANTransceiverTest, testRemoveCANFrameListener)
{
    tBitFieldListener listener;
    // add listener the first time
    fpTransceiver->addCANFrameListener(listener);
    // add listener again
    fpTransceiver->addCANFrameListener(listener);
    // now remove listener
    fpTransceiver->removeCANFrameListener(listener);
    // add listener the again
    fpTransceiver->addCANFrameListener(listener);
    // now remove listener again
    fpTransceiver->removeCANFrameListener(listener);

    // the same for an IntervalListener
    tIntervalListener listener2;
    // add listener the first time
    fpTransceiver->addCANFrameListener(listener2);
    // add listener again
    fpTransceiver->addCANFrameListener(listener2);
    // now remove listener
    fpTransceiver->removeCANFrameListener(listener2);
    // add listener the again
    fpTransceiver->addCANFrameListener(listener2);
    // now remove listener again
    fpTransceiver->removeCANFrameListener(listener2);
    // add VIP listener
    fpTransceiver->addVIPCANFrameListener(listener2);
    // now remove listener again
    fpTransceiver->removeCANFrameListener(listener2);
}

/**
 * @test
 * verification of receive method
 */
TEST_F(AbstractCANTransceiverTest, testStateListeners)
{
    tStateChangeListener stateListener;
    ICanTransceiver::ErrorCode status;
    EXPECT_CALL(*fpTransceiver, init()).Times(1);
    // init transceiver
    status = fpTransceiver->init();
    ASSERT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, status);
    ASSERT_EQ(ICanTransceiver::State::CLOSED, fpTransceiver->getState());
    fpTransceiver->setStateListener(stateListener);
    EXPECT_CALL(stateListener, canTransceiverStateChanged(_, _)).Times(1);
    EXPECT_CALL(*fpTransceiver, open()).Times(1);
    EXPECT_CALL(stateListener, phyErrorOccurred(_)).Times(1);
    status = fpTransceiver->open();
    ASSERT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, status);
    ASSERT_EQ(ICanTransceiver::State::OPEN, fpTransceiver->getState());
    //        EXPECT_CALL(*fpTransceiver, getCANTransceiverState()).Times(1);
    ASSERT_EQ(
        ICANTransceiverStateListener::CANTransceiverState::ACTIVE,
        fpTransceiver->getCANTransceiverState());
    fpTransceiver->setTransceiverState2(
        can::ICANTransceiverStateListener::CANTransceiverState::BUS_OFF);
    fpTransceiver->removeStateListener();
    fpTransceiver->setTransceiverState2(
        can::ICANTransceiverStateListener::CANTransceiverState::ACTIVE);
}

TEST_F(AbstractCANTransceiverTest, testSendListeners)
{
    tStateChangeListener sendListener;
    CANFrame frame(0x555);
    EXPECT_CALL(*fpTransceiver, write(_, _)).Times(1);
    EXPECT_CALL(sendListener, canFrameSent(_)).Times(1);
    fpTransceiver->write(frame, sendListener);
    tSendFrameListener sendListener2;
    fpTransceiver->setCANFrameSentListener(&sendListener2);
    EXPECT_CALL(*fpTransceiver, write(_)).Times(1);
    fpTransceiver->write(frame);
    fpTransceiver->setCANFrameSentListener(nullptr);
}

/**
 * @test
 * verification of receive method
 */
TEST_F(AbstractCANTransceiverTest, testNotifyListeners)
{
    ICanTransceiver::ErrorCode status;
    EXPECT_CALL(*fpTransceiver, init()).Times(1);
    // init transceiver
    status = fpTransceiver->init();
    ASSERT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, status);
    ASSERT_EQ(ICanTransceiver::State::CLOSED, fpTransceiver->getState());

    // build CANFrame
    uint8_t payload[6]  = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
    uint8_t payload1[6] = {0x06, 0x05, 0x04, 0x03, 0x02, 0x01};
    CANFrame frame(0x555, payload, 6);
    CANFrame frame1(0x123, payload1, 6);
    // init listeners
    tBitFieldListener listener1;
    tBitFieldListener listener2;
    tIntervalListener listener3;
    tIntervalListener listener4;
    tAbstractBitFieldFilterListener listener5;
    // listener1 shall receive the frame
    listener1.getFilter().add(0x555);
    listener1.addExpectedCANFrame(frame);
    // listener3 too
    listener3.getFilter().add(0x000, 0x7FF);
    listener3.addExpectedCANFrame(frame);
    listener4.getFilter().add(0x000, 0x7FF);
    listener4.addExpectedCANFrame(frame1);
    listener5.getFilter().add(0x555);
    listener5.getFilter().add(0, 0x700);
    listener5.addExpectedCANFrame(frame);

    // test path when transceiver is still closed
    fpTransceiver->addCANFrameListener(listener3);
    fpTransceiver->inject(frame);
    listener2.verify();
    // open transceiver
    EXPECT_CALL(*fpTransceiver, open()).Times(1);
    status = fpTransceiver->open();
    ASSERT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, status);
    ASSERT_EQ(ICanTransceiver::State::OPEN, fpTransceiver->getState());
    // add listeners to transceiver
    fpTransceiver->addCANFrameListener(listener1);
    fpTransceiver->addCANFrameListener(listener2);
    fpTransceiver->addCANFrameListener(listener4);
    fpTransceiver->addCANFrameListener(listener5);
    EXPECT_CALL(listener1, frameReceived(_)).Times(1);
    // not called for listener2, because filter excludes the frame ID
    //        EXPECT_CALL(listener2, frameReceived(_)).Times(1);
    EXPECT_CALL(listener3, frameReceived(_)).Times(2);
    EXPECT_CALL(listener4, frameReceived(_)).Times(1);
    EXPECT_CALL(listener5, frameReceived(_)).Times(2);
    fpTransceiver->inject(frame);

    listener1.verify();
    listener2.verify();
    listener3.verify();
    listener5.verify();

    // called 2 times since already called on first inject
    EXPECT_CALL(listener4, frameReceived(_)).Times(1);
    fpTransceiver->inject(frame1);

    listener4.verify();

    frame.setId(0x17000080);
    //        EXPECT_CALL(listener5, frameReceived(_)).Times(2);
    fpTransceiver->inject(frame);

    listener5.getFilter().clear();
    listener5.getFilter().add(0x555);
    ASSERT_EQ(true, listener5.compareFilter(0x555, 0xFF));
    ASSERT_NE(true, listener5.compareFilter(0x555, 0));
    listener5.getFilter().clear();

    fpTransceiver->removeCANFrameListener(listener1);
    fpTransceiver->removeCANFrameListener(listener2);
    fpTransceiver->removeCANFrameListener(listener3);
    fpTransceiver->removeCANFrameListener(listener4);
    fpTransceiver->removeCANFrameListener(listener5);
}

TEST_F(AbstractCANTransceiverTest, testNotifySentListeners)
{
    uint8_t payload[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
    CANFrame frame(0x555, payload, 6);

    tCanTransceiver transceiver;
    transceiver.notifySentListeners(frame);

    FilteredCANFrameSentListenerMock listener1;
    FilteredCANFrameSentListenerMock listener2;

    transceiver.addCANFrameSentListener(listener1);
    transceiver.addCANFrameSentListener(listener2);
    EXPECT_CALL(listener1, canFrameSent(Ref(frame)));
    EXPECT_CALL(listener2, canFrameSent(Ref(frame)));
    EXPECT_CALL(fSystemTimer, getSystemTimeUs32Bit()).WillOnce(Return(150U));
    transceiver.notifySentListeners(frame);
    EXPECT_EQ(150U, frame.timestamp());

    transceiver.removeCANFrameSentListener(listener1);
    EXPECT_CALL(listener2, canFrameSent(Ref(frame)));
    EXPECT_CALL(fSystemTimer, getSystemTimeUs32Bit()).WillOnce(Return(180U));
    transceiver.notifySentListeners(frame);
    EXPECT_EQ(180U, frame.timestamp());
    transceiver.removeCANFrameSentListener(listener2);
}

TEST_F(AbstractCANTransceiverTest, testSetFrameSentListener)
{
    uint8_t payload[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
    CANFrame frame(0x555, payload, 6);

    tCanTransceiver transceiver;
    FilteredCANFrameSentListenerMock listener1;
    transceiver.setCANFrameSentListener(nullptr);
    transceiver.setCANFrameSentListener(&listener1);
    EXPECT_CALL(listener1, canFrameSent(Ref(frame)));
    EXPECT_CALL(fSystemTimer, getSystemTimeUs32Bit()).WillOnce(Return(150U));
    transceiver.notifySentListeners(frame);
    EXPECT_EQ(150U, frame.timestamp());

    transceiver.setCANFrameSentListener(nullptr);
    transceiver.notifySentListeners(frame);
}

} // anonymous namespace
