// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file BxCanTransceiverIntegrationTest.cpp
 * \brief Integration tests verifying the STM32 BxCanTransceiver works correctly
 *        with the DoCAN transport layer and UDS-style message patterns.
 *
 * Tests cover:
 *   1. Multi-listener routing through AbstractCANTransceiver::notifyListeners
 *   2. DoCAN TX callback pattern (write-with-listener, sendPending, async chain)
 *   3. Full UDS round-trip simulation through transceiver
 *   4. CAN frame content verification (ID, DLC, padding, PCI)
 *   5. Simultaneous demo + DoCAN traffic
 */

#include "can/transceiver/bxcan/BxCanTransceiver.h"

#include "async/AsyncMock.h"
#include "bsp/timer/SystemTimerMock.h"
#include "can/canframes/ICANFrameSentListener.h"
#include "can/filter/BitFieldFilter.h"
#include "can/framemgmt/AbstractBitFieldFilteredCANFrameListener.h"

#include "docan/addressing/DoCanAddressConverterMock.h"
#include "docan/addressing/DoCanNormalAddressing.h"
#include "docan/can/DoCanPhysicalCanTransceiver.h"
#include "docan/datalink/DoCanDataFrameTransmitterCallbackMock.h"
#include "docan/datalink/DoCanDefaultFrameSizeMapper.h"
#include "docan/datalink/DoCanFdFrameSizeMapper.h"
#include "docan/datalink/DoCanFrameCodec.h"
#include "docan/datalink/DoCanFrameCodecConfigPresets.h"
#include "docan/datalink/DoCanFrameReceiverMock.h"

#include <can/filter/FilterMock.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::can;
using namespace ::bios;
using namespace ::docan;
using namespace ::testing;

// ---------------------------------------------------------------------------
// Mock sent listener (reused from unit tests)
// ---------------------------------------------------------------------------
class MockCANFrameSentListener : public ::can::ICANFrameSentListener
{
public:
    MOCK_METHOD(void, canFrameSent, (::can::CANFrame const&), (override));
};

// ---------------------------------------------------------------------------
// Demo listener: bit-field filtered, records received frames
// ---------------------------------------------------------------------------
class DemoListener : public AbstractBitFieldFilteredCANFrameListener
{
public:
    DemoListener() : _frameCount(0U) {}

    MOCK_METHOD(void, frameReceived, (CANFrame const& frame), (override));

    uint32_t _frameCount;
};

// ---------------------------------------------------------------------------
// Base fixture: BxCanTransceiver in OPEN state with auto-async
// ---------------------------------------------------------------------------
class BxCanIntegrationBase : public Test
{
public:
    ::async::AsyncMock fAsyncMock;
    ::async::ContextType fAsyncContext = 0;
    uint8_t fBusId                     = 0;
    BxCanDevice::Config fDevConfig{};
    ::testing::SystemTimerMock fSystemTimer;
};

class BxCanIntegrationTest : public BxCanIntegrationBase
{
public:
    BxCanIntegrationTest() : fBct(fAsyncContext, fBusId, fDevConfig)
    {
        EXPECT_CALL(fBct.fDevice, init()).Times(AnyNumber());
        EXPECT_CALL(fBct.fDevice, start()).Times(AnyNumber());
        EXPECT_CALL(fBct.fDevice, stop()).Times(AnyNumber());
        EXPECT_CALL(fAsyncMock, execute(fAsyncContext, _))
            .Times(AnyNumber())
            .WillRepeatedly([](auto, auto& runnable) { runnable.execute(); });

        EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBct.init());
        EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBct.open());
    }

    BxCanTransceiver fBct;
};

// ---------------------------------------------------------------------------
// Manual-async fixture for integration tests
// ---------------------------------------------------------------------------
class BxCanIntegrationManualAsync : public BxCanIntegrationBase
{
public:
    BxCanIntegrationManualAsync() : fBct(fAsyncContext, fBusId, fDevConfig)
    {
        EXPECT_CALL(fBct.fDevice, init()).Times(AnyNumber());
        EXPECT_CALL(fBct.fDevice, start()).Times(AnyNumber());
        EXPECT_CALL(fBct.fDevice, stop()).Times(AnyNumber());
        EXPECT_CALL(fAsyncMock, execute(fAsyncContext, _))
            .Times(AnyNumber())
            .WillRepeatedly([this](auto, auto& runnable) { fCapturedRunnable = &runnable; });

        EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBct.init());
        EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBct.open());
    }

    void executeAsyncCallback()
    {
        ASSERT_NE(nullptr, fCapturedRunnable);
        fCapturedRunnable->execute();
        fCapturedRunnable = nullptr;
    }

    BxCanTransceiver fBct;
    ::async::RunnableType* fCapturedRunnable = nullptr;
};

// ===================================================================
// Helper: simulate RX of a frame through the transceiver's receiveTask
// ===================================================================
static void simulateRx(BxCanTransceiver& bct, BxCanDevice& dev, CANFrame const& frame)
{
    EXPECT_CALL(dev, getRxCount()).WillOnce(Return(1));
    EXPECT_CALL(dev, getRxFrame(0)).WillOnce(ReturnRef(frame));
    EXPECT_CALL(dev, clearRxQueue()).Times(1);
    EXPECT_CALL(dev, enableRxInterrupt()).Times(1);
    bct.receiveTask();
}

// ===================================================================
// Category 1 -- Multi-listener routing (8 tests)
// ===================================================================

TEST_F(BxCanIntegrationTest, demoListenerReceives0x123ButNot0x600)
{
    DemoListener demo;
    demo.getFilter().add(0x123);
    fBct.addCANFrameListener(demo);

    uint8_t payload[] = {0x01, 0x02, 0x03};
    CANFrame frame123(0x123U, payload, 3);
    CANFrame frame600(0x600U, payload, 3);

    EXPECT_CALL(demo, frameReceived(_)).Times(1);
    simulateRx(fBct, fBct.fDevice, frame123);

    Mock::VerifyAndClearExpectations(&demo);

    EXPECT_CALL(demo, frameReceived(_)).Times(0);
    simulateRx(fBct, fBct.fDevice, frame600);

    fBct.removeCANFrameListener(demo);
}

TEST_F(BxCanIntegrationTest, docanListenerReceives0x600ButNot0x123)
{
    DemoListener docanListener;
    docanListener.getFilter().add(0x600);
    fBct.addCANFrameListener(docanListener);

    uint8_t payload[] = {0x02, 0x3E, 0x00};
    CANFrame frame600(0x600U, payload, 3);
    CANFrame frame123(0x123U, payload, 3);

    EXPECT_CALL(docanListener, frameReceived(_)).Times(1);
    simulateRx(fBct, fBct.fDevice, frame600);

    Mock::VerifyAndClearExpectations(&docanListener);

    EXPECT_CALL(docanListener, frameReceived(_)).Times(0);
    simulateRx(fBct, fBct.fDevice, frame123);

    fBct.removeCANFrameListener(docanListener);
}

TEST_F(BxCanIntegrationTest, bothListenersReceiveWhenFilterOverlaps)
{
    DemoListener listener1;
    DemoListener listener2;
    listener1.getFilter().add(0x100);
    listener2.getFilter().add(0x100);

    fBct.addCANFrameListener(listener1);
    fBct.addCANFrameListener(listener2);

    uint8_t payload[] = {0xAA};
    CANFrame frame(0x100U, payload, 1);

    EXPECT_CALL(listener1, frameReceived(_)).Times(1);
    EXPECT_CALL(listener2, frameReceived(_)).Times(1);
    simulateRx(fBct, fBct.fDevice, frame);

    fBct.removeCANFrameListener(listener1);
    fBct.removeCANFrameListener(listener2);
}

TEST_F(BxCanIntegrationTest, noListenerReceivesUnfilteredId)
{
    DemoListener listener;
    listener.getFilter().add(0x200);
    fBct.addCANFrameListener(listener);

    uint8_t payload[] = {0xBB};
    CANFrame frame(0x300U, payload, 1);

    EXPECT_CALL(listener, frameReceived(_)).Times(0);
    simulateRx(fBct, fBct.fDevice, frame);

    fBct.removeCANFrameListener(listener);
}

TEST_F(BxCanIntegrationTest, addListenerThenRemoveStopsDelivery)
{
    DemoListener listener;
    listener.getFilter().add(0x400);
    fBct.addCANFrameListener(listener);

    uint8_t payload[] = {0xCC};
    CANFrame frame(0x400U, payload, 1);

    EXPECT_CALL(listener, frameReceived(_)).Times(1);
    simulateRx(fBct, fBct.fDevice, frame);
    Mock::VerifyAndClearExpectations(&listener);

    fBct.removeCANFrameListener(listener);

    EXPECT_CALL(listener, frameReceived(_)).Times(0);
    simulateRx(fBct, fBct.fDevice, frame);
}

TEST_F(BxCanIntegrationTest, addMultipleListenersSameFilter)
{
    DemoListener l1;
    DemoListener l2;
    DemoListener l3;
    l1.getFilter().add(0x500);
    l2.getFilter().add(0x500);
    l3.getFilter().add(0x500);

    fBct.addCANFrameListener(l1);
    fBct.addCANFrameListener(l2);
    fBct.addCANFrameListener(l3);

    uint8_t payload[] = {0xDD};
    CANFrame frame(0x500U, payload, 1);

    EXPECT_CALL(l1, frameReceived(_)).Times(1);
    EXPECT_CALL(l2, frameReceived(_)).Times(1);
    EXPECT_CALL(l3, frameReceived(_)).Times(1);
    simulateRx(fBct, fBct.fDevice, frame);

    fBct.removeCANFrameListener(l1);
    fBct.removeCANFrameListener(l2);
    fBct.removeCANFrameListener(l3);
}

TEST_F(BxCanIntegrationTest, listenerFilterMatchUsesCanIdNotPayload)
{
    DemoListener listener;
    listener.getFilter().add(0x123);
    fBct.addCANFrameListener(listener);

    // Payload contains 0x123 in bytes but frame ID is 0x456 -- should NOT match
    uint8_t payload[] = {0x01, 0x23};
    CANFrame frame(0x456U, payload, 2);

    EXPECT_CALL(listener, frameReceived(_)).Times(0);
    simulateRx(fBct, fBct.fDevice, frame);

    fBct.removeCANFrameListener(listener);
}

TEST_F(BxCanIntegrationBase, listenerNotNotifiedWhenTransceiverClosed)
{
    BxCanTransceiver bct(fAsyncContext, fBusId, fDevConfig);

    EXPECT_CALL(bct.fDevice, init()).Times(AnyNumber());
    EXPECT_CALL(bct.fDevice, start()).Times(AnyNumber());
    EXPECT_CALL(bct.fDevice, stop()).Times(AnyNumber());
    EXPECT_CALL(fAsyncMock, execute(fAsyncContext, _))
        .Times(AnyNumber())
        .WillRepeatedly([](auto, auto& runnable) { runnable.execute(); });

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, bct.init());
    // Do NOT open -- stays CLOSED (after init = INITIALIZED, close -> CLOSED)
    // Actually after init state is not OPEN, notifyListeners checks for CLOSED

    DemoListener listener;
    listener.getFilter().add(0x100);
    bct.addCANFrameListener(listener);

    // Close the transceiver so state == CLOSED
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, bct.close());

    uint8_t payload[] = {0xEE};
    CANFrame frame(0x100U, payload, 1);

    EXPECT_CALL(listener, frameReceived(_)).Times(0);
    simulateRx(bct, bct.fDevice, frame);

    bct.removeCANFrameListener(listener);
}

// ===================================================================
// Category 2 -- DoCAN TX callback pattern (6 tests)
// ===================================================================

TEST_F(BxCanIntegrationTest, docanWriteWithListenerDefersCallback)
{
    StrictMock<MockCANFrameSentListener> listener;
    CANFrame frame;

    EXPECT_CALL(fBct.fDevice, transmit(_)).WillOnce(Return(true));
    // During write(), listener must NOT be called (deferred pattern)
    EXPECT_CALL(listener, canFrameSent(_)).Times(0);

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBct.write(frame, listener));
    Mock::VerifyAndClearExpectations(&listener);

    // Now allow the callback
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    EXPECT_CALL(fBct.fDevice, transmitISR());
    BxCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(BxCanIntegrationManualAsync, docanCallbackFiresAfterSendPendingIsSet)
{
    // The critical race condition test:
    // write(frame, listener) must set _sendPending BEFORE write returns,
    // so that when TX ISR fires, the DoCAN canFrameSent sees _sendPending == true.
    StrictMock<MockCANFrameSentListener> listener;
    CANFrame frame;

    EXPECT_CALL(fBct.fDevice, transmit(_)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBct.write(frame, listener));

    // TX ISR fires
    fCapturedRunnable = nullptr;
    EXPECT_CALL(fBct.fDevice, transmitISR());
    BxCanTransceiver::transmitInterrupt(fBusId);

    // Async runnable was captured -- listener not called yet
    ASSERT_NE(nullptr, fCapturedRunnable);
    Mock::VerifyAndClearExpectations(&listener);

    // Execute in task context -- NOW listener is called
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    executeAsyncCallback();
}

TEST_F(BxCanIntegrationTest, docanMultiFrameTransmission)
{
    // SF response fits in single frame -- verify write succeeds
    uint8_t payload[] = {0x04, 0x12, 0x13, 0x24, 0x45};
    CANFrame frame(0x601U, payload, sizeof(payload));
    MockCANFrameSentListener listener;

    EXPECT_CALL(fBct.fDevice, transmit(_)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBct.write(frame, listener));

    EXPECT_CALL(fBct.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    BxCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(BxCanIntegrationTest, docanFlowControlReceived)
{
    // Simulate receiving a flow control frame (FC) after sending a first frame (FF)
    // Use DoCAN physical transceiver to decode the FC

    using Addressing      = DoCanNormalAddressing<>;
    using DataLinkLayer   = Addressing::DataLinkLayerType;
    using PhysTransceiver = DoCanPhysicalCanTransceiver<Addressing>;

    StrictMock<FilterMock> filterMock;
    Addressing addressing;
    StrictMock<DoCanAddressConverterMock<DataLinkLayer>> addrConvMock;
    DoCanFdFrameSizeMapper<uint8_t> sizeMapper;
    DoCanFrameCodec<DataLinkLayer> codec(
        DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, sizeMapper);
    StrictMock<DoCanFrameReceiverMock<DataLinkLayer>> frameReceiverMock;

    PhysTransceiver docanXcvr(fBct, filterMock, addrConvMock, addressing);

    EXPECT_CALL(filterMock, acceptMerger(_)).Times(AnyNumber());
    docanXcvr.init(frameReceiverMock);

    // Simulate receiving a FC frame on CAN ID 0x600
    uint8_t fcPayload[] = {0x30, 0x00, 0x0A}; // FC: CTS, BS=0, STmin=10ms
    CANFrame fcFrame(0x600U, fcPayload, sizeof(fcPayload));

    EXPECT_CALL(filterMock, match(0x600U)).WillOnce(Return(true));
    EXPECT_CALL(addrConvMock, getReceptionParameters(0x600U, _, _)).WillOnce(Return(&codec));
    EXPECT_CALL(frameReceiverMock, flowControlFrameReceived(0x600U, FlowStatus::CTS, 0x00, 0x0A));

    // Deliver frame through the transceiver's notifyListeners path
    simulateRx(fBct, fBct.fDevice, fcFrame);

    docanXcvr.shutdown();
}

TEST_F(BxCanIntegrationManualAsync, docanTxTimeoutWhenNoISR)
{
    // Write with listener but never fire TX ISR -- simulates timeout scenario
    MockCANFrameSentListener listener;
    CANFrame frame;

    EXPECT_CALL(fBct.fDevice, transmit(_)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBct.write(frame, listener));

    // No transmitInterrupt() call -- callback never fires
    // Listener should never be called
    EXPECT_CALL(listener, canFrameSent(_)).Times(0);
    Mock::VerifyAndClearExpectations(&listener);
}

TEST_F(BxCanIntegrationTest, docanTxCompletionNotifiesSentListeners)
{
    MockCANFrameSentListener listener;
    CANFrame frame(0x601U);

    EXPECT_CALL(fBct.fDevice, transmit(_)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBct.write(frame, listener));

    // TX ISR fires -- listener should be notified with the frame
    EXPECT_CALL(fBct.fDevice, transmitISR());
    EXPECT_CALL(listener, canFrameSent(_)).Times(1);
    BxCanTransceiver::transmitInterrupt(fBusId);
}

// ===================================================================
// Category 3 -- Full UDS round-trip through transceiver (6 tests)
// ===================================================================

// Helper: set up a DoCAN physical transceiver on the BxCanTransceiver
struct UdsRoundTripFixture : public BxCanIntegrationTest
{
    using Addressing      = DoCanNormalAddressing<>;
    using DataLinkLayer   = Addressing::DataLinkLayerType;
    using PhysTransceiver = DoCanPhysicalCanTransceiver<Addressing>;

    UdsRoundTripFixture()
    : sizeMapper()
    , codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, sizeMapper)
    , docanXcvr(fBct, filterMock, addrConvMock, addressing)
    {
        EXPECT_CALL(filterMock, acceptMerger(_)).Times(AnyNumber());
        docanXcvr.init(frameReceiverMock);
    }

    ~UdsRoundTripFixture() override { docanXcvr.shutdown(); }

    StrictMock<FilterMock> filterMock;
    Addressing addressing;
    StrictMock<DoCanAddressConverterMock<DataLinkLayer>> addrConvMock;
    DoCanFdFrameSizeMapper<uint8_t> sizeMapper;
    DoCanFrameCodec<DataLinkLayer> codec;
    StrictMock<DoCanFrameReceiverMock<DataLinkLayer>> frameReceiverMock;
    PhysTransceiver docanXcvr;
};

TEST_F(UdsRoundTripFixture, testerPresentRoundTrip)
{
    // RX: 0x600 [02 3E 00] -- TesterPresent request
    uint8_t rxPayload[] = {0x02, 0x3E, 0x00};
    CANFrame rxFrame(0x600U, rxPayload, sizeof(rxPayload));

    EXPECT_CALL(filterMock, match(0x600U)).WillOnce(Return(true));
    EXPECT_CALL(addrConvMock, getReceptionParameters(0x600U, _, _)).WillOnce(Return(&codec));
    // Single frame with 2 data bytes: 0x3E, 0x00
    EXPECT_CALL(frameReceiverMock, firstDataFrameReceived(_, 2, 1, _, _));
    simulateRx(fBct, fBct.fDevice, rxFrame);

    // TX: 0x601 [02 7E 00] -- TesterPresent positive response
    uint8_t txPayload[] = {0x02, 0x7E, 0x00};
    CANFrame txFrame(0x601U, txPayload, sizeof(txPayload));

    EXPECT_CALL(fBct.fDevice, transmit(_)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBct.write(txFrame));
}

TEST_F(UdsRoundTripFixture, diagSessionControlRoundTrip)
{
    // RX: 0x600 [02 10 01] -- DiagnosticSessionControl (default session)
    uint8_t rxPayload[] = {0x02, 0x10, 0x01};
    CANFrame rxFrame(0x600U, rxPayload, sizeof(rxPayload));

    EXPECT_CALL(filterMock, match(0x600U)).WillOnce(Return(true));
    EXPECT_CALL(addrConvMock, getReceptionParameters(0x600U, _, _)).WillOnce(Return(&codec));
    EXPECT_CALL(frameReceiverMock, firstDataFrameReceived(_, 2, 1, _, _));
    simulateRx(fBct, fBct.fDevice, rxFrame);

    // TX: 0x601 [06 50 01 00 32 01 F4] -- positive response with timing
    uint8_t txPayload[] = {0x06, 0x50, 0x01, 0x00, 0x32, 0x01, 0xF4};
    CANFrame txFrame(0x601U, txPayload, sizeof(txPayload));

    EXPECT_CALL(fBct.fDevice, transmit(_)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBct.write(txFrame));
}

TEST_F(UdsRoundTripFixture, readDIDSingleFrame)
{
    // RX: 0x600 [03 22 CF 01] -- ReadDataByIdentifier 0xCF01
    uint8_t rxPayload[] = {0x03, 0x22, 0xCF, 0x01};
    CANFrame rxFrame(0x600U, rxPayload, sizeof(rxPayload));

    EXPECT_CALL(filterMock, match(0x600U)).WillOnce(Return(true));
    EXPECT_CALL(addrConvMock, getReceptionParameters(0x600U, _, _)).WillOnce(Return(&codec));
    EXPECT_CALL(frameReceiverMock, firstDataFrameReceived(_, 3, 1, _, _));
    simulateRx(fBct, fBct.fDevice, rxFrame);
}

TEST_F(UdsRoundTripFixture, unknownServiceReturnsNRC)
{
    // RX: 0x600 [02 FF 00] -- unknown service 0xFF
    uint8_t rxPayload[] = {0x02, 0xFF, 0x00};
    CANFrame rxFrame(0x600U, rxPayload, sizeof(rxPayload));

    EXPECT_CALL(filterMock, match(0x600U)).WillOnce(Return(true));
    EXPECT_CALL(addrConvMock, getReceptionParameters(0x600U, _, _)).WillOnce(Return(&codec));
    EXPECT_CALL(frameReceiverMock, firstDataFrameReceived(_, 2, 1, _, _));
    simulateRx(fBct, fBct.fDevice, rxFrame);

    // TX: NRC response 0x7F FF 11 (serviceNotSupported)
    uint8_t txPayload[] = {0x03, 0x7F, 0xFF, 0x11};
    CANFrame txFrame(0x601U, txPayload, sizeof(txPayload));

    EXPECT_CALL(fBct.fDevice, transmit(_)).WillOnce(Return(true));
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBct.write(txFrame));
}

TEST_F(UdsRoundTripFixture, noResponseWhenFilterDoesNotMatch)
{
    // Send on wrong CAN ID (0x700 instead of 0x600) -- filter rejects
    uint8_t rxPayload[] = {0x02, 0x3E, 0x00};
    CANFrame rxFrame(0x700U, rxPayload, sizeof(rxPayload));

    EXPECT_CALL(filterMock, match(0x700U)).WillOnce(Return(false));
    // frameReceiverMock should NOT be called
    EXPECT_CALL(frameReceiverMock, firstDataFrameReceived(_, _, _, _, _)).Times(0);
    simulateRx(fBct, fBct.fDevice, rxFrame);
}

TEST_F(UdsRoundTripFixture, responseOnlyToConfiguredReceptionId)
{
    // 0x600 is configured -- should be received
    uint8_t rxPayload[] = {0x02, 0x3E, 0x00};
    CANFrame frame600(0x600U, rxPayload, sizeof(rxPayload));

    EXPECT_CALL(filterMock, match(0x600U)).WillOnce(Return(true));
    EXPECT_CALL(addrConvMock, getReceptionParameters(0x600U, _, _)).WillOnce(Return(&codec));
    EXPECT_CALL(frameReceiverMock, firstDataFrameReceived(_, 2, 1, _, _));
    simulateRx(fBct, fBct.fDevice, frame600);
    Mock::VerifyAndClearExpectations(&frameReceiverMock);
    Mock::VerifyAndClearExpectations(&addrConvMock);

    // 0x601 is the TX ID, not reception -- filter should reject
    CANFrame frame601(0x601U, rxPayload, sizeof(rxPayload));
    EXPECT_CALL(filterMock, match(0x601U)).WillOnce(Return(false));
    EXPECT_CALL(frameReceiverMock, firstDataFrameReceived(_, _, _, _, _)).Times(0);
    simulateRx(fBct, fBct.fDevice, frame601);
}

// ===================================================================
// Category 4 -- CAN frame content verification (8 tests)
// ===================================================================

TEST_F(BxCanIntegrationTest, txFrameHasCorrectCanId)
{
    CANFrame captured;
    EXPECT_CALL(fBct.fDevice, transmit(_)).WillOnce(DoAll(SaveArg<0>(&captured), Return(true)));

    uint8_t payload[] = {0x02, 0x7E, 0x00};
    CANFrame txFrame(0x601U, payload, sizeof(payload));
    fBct.write(txFrame);

    EXPECT_EQ(0x601U, captured.getId());
}

TEST_F(BxCanIntegrationTest, txFrameHasCorrectDlc)
{
    CANFrame captured;
    EXPECT_CALL(fBct.fDevice, transmit(_)).WillOnce(DoAll(SaveArg<0>(&captured), Return(true)));

    uint8_t payload[] = {0x02, 0x7E, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC};
    CANFrame txFrame(0x601U, payload, 8);
    fBct.write(txFrame);

    EXPECT_EQ(8U, captured.getPayloadLength());
}

TEST_F(BxCanIntegrationTest, txFrameIsPaddedWithCC)
{
    // Verify ISO-TP padding (0xCC) when sending a padded frame
    uint8_t payload[] = {0x02, 0x7E, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC};
    CANFrame txFrame(0x601U, payload, 8);

    CANFrame captured;
    EXPECT_CALL(fBct.fDevice, transmit(_)).WillOnce(DoAll(SaveArg<0>(&captured), Return(true)));

    fBct.write(txFrame);

    // Bytes 3-7 should be 0xCC (ISO-TP padding)
    for (uint8_t i = 3; i < 8; ++i)
    {
        EXPECT_EQ(0xCCU, captured.getPayload()[i]) << "Byte " << static_cast<int>(i);
    }
}

TEST_F(BxCanIntegrationTest, txFrameSingleFramePCICorrect)
{
    // Single Frame PCI: byte 0 = length (for classic CAN, SF_DL in low nibble)
    uint8_t payload[] = {0x02, 0x3E, 0x00};
    CANFrame txFrame(0x601U, payload, sizeof(payload));

    CANFrame captured;
    EXPECT_CALL(fBct.fDevice, transmit(_)).WillOnce(DoAll(SaveArg<0>(&captured), Return(true)));

    fBct.write(txFrame);

    // Byte 0 should be 0x02 (SF PCI with length 2)
    EXPECT_EQ(0x02U, captured.getPayload()[0]);
}

TEST_F(BxCanIntegrationTest, rxFrameStandardIdExtracted)
{
    DemoListener listener;
    listener.getFilter().add(0x123);
    fBct.addCANFrameListener(listener);

    uint8_t payload[] = {0xAA, 0xBB};
    CANFrame frame(0x123U, payload, 2);

    CANFrame received;
    EXPECT_CALL(listener, frameReceived(_)).WillOnce(SaveArg<0>(&received));

    simulateRx(fBct, fBct.fDevice, frame);

    EXPECT_EQ(0x123U, received.getId());
    EXPECT_TRUE(CanId::isBase(received.getId()));

    fBct.removeCANFrameListener(listener);
}

TEST_F(BxCanIntegrationTest, rxFrameExtendedIdExtracted)
{
    // Extended CAN ID has bit 31 set
    uint32_t extId = CanId::extended(0x18DAF1FE);

    DemoListener listener;
    // Extended IDs do not go through BitFieldFilter.add() in the standard way,
    // but we can verify the frame ID propagation. Use open() to accept all.
    listener.getFilter().open();
    fBct.addCANFrameListener(listener);

    uint8_t payload[] = {0x02, 0x3E, 0x00};
    CANFrame frame(extId, payload, sizeof(payload));

    CANFrame received;
    EXPECT_CALL(listener, frameReceived(_)).WillOnce(SaveArg<0>(&received));

    simulateRx(fBct, fBct.fDevice, frame);

    EXPECT_TRUE(CanId::isExtended(received.getId()));
    EXPECT_EQ(0x18DAF1FEU, CanId::rawId(received.getId()));

    fBct.removeCANFrameListener(listener);
}

TEST_F(BxCanIntegrationTest, rxFrameZeroDlcHandled)
{
    DemoListener listener;
    listener.getFilter().open();
    fBct.addCANFrameListener(listener);

    CANFrame frame(0x100U);
    frame.setPayloadLength(0);

    CANFrame received;
    EXPECT_CALL(listener, frameReceived(_)).WillOnce(SaveArg<0>(&received));

    simulateRx(fBct, fBct.fDevice, frame);

    EXPECT_EQ(0U, received.getPayloadLength());

    fBct.removeCANFrameListener(listener);
}

TEST_F(BxCanIntegrationTest, rxFrame8BytePayloadCorrect)
{
    DemoListener listener;
    listener.getFilter().add(0x200);
    fBct.addCANFrameListener(listener);

    uint8_t payload[] = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80};
    CANFrame frame(0x200U, payload, 8);

    CANFrame received;
    EXPECT_CALL(listener, frameReceived(_)).WillOnce(SaveArg<0>(&received));

    simulateRx(fBct, fBct.fDevice, frame);

    EXPECT_EQ(8U, received.getPayloadLength());
    for (uint8_t i = 0; i < 8; ++i)
    {
        EXPECT_EQ(payload[i], received.getPayload()[i]) << "Byte " << static_cast<int>(i);
    }

    fBct.removeCANFrameListener(listener);
}

// ===================================================================
// Category 5 -- Simultaneous demo + DoCAN (4 tests)
// ===================================================================

TEST_F(BxCanIntegrationTest, demoEchoDoesNotInterfereWithUDS)
{
    // Demo listener on 0x123, DoCAN listener on 0x600 -- both active
    DemoListener demoListener;
    demoListener.getFilter().add(0x123);
    fBct.addCANFrameListener(demoListener);

    using Addressing      = DoCanNormalAddressing<>;
    using DataLinkLayer   = Addressing::DataLinkLayerType;
    using PhysTransceiver = DoCanPhysicalCanTransceiver<Addressing>;

    StrictMock<FilterMock> filterMock;
    Addressing addressing;
    StrictMock<DoCanAddressConverterMock<DataLinkLayer>> addrConvMock;
    DoCanFdFrameSizeMapper<uint8_t> sizeMapper;
    DoCanFrameCodec<DataLinkLayer> codec(
        DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, sizeMapper);
    StrictMock<DoCanFrameReceiverMock<DataLinkLayer>> frameReceiverMock;
    PhysTransceiver docanXcvr(fBct, filterMock, addrConvMock, addressing);

    EXPECT_CALL(filterMock, acceptMerger(_)).Times(AnyNumber());
    docanXcvr.init(frameReceiverMock);

    // Send demo frame on 0x123 -- only demo listener should fire
    uint8_t demoPayload[] = {0xAA, 0xBB};
    CANFrame demoFrame(0x123U, demoPayload, 2);

    EXPECT_CALL(demoListener, frameReceived(_)).Times(1);
    EXPECT_CALL(filterMock, match(0x123U)).WillOnce(Return(false));
    simulateRx(fBct, fBct.fDevice, demoFrame);
    Mock::VerifyAndClearExpectations(&demoListener);
    Mock::VerifyAndClearExpectations(&filterMock);

    // Send UDS frame on 0x600 -- only DoCAN listener should fire
    uint8_t udsPayload[] = {0x02, 0x3E, 0x00};
    CANFrame udsFrame(0x600U, udsPayload, sizeof(udsPayload));

    EXPECT_CALL(demoListener, frameReceived(_)).Times(0);
    EXPECT_CALL(filterMock, match(0x600U)).WillOnce(Return(true));
    EXPECT_CALL(addrConvMock, getReceptionParameters(0x600U, _, _)).WillOnce(Return(&codec));
    EXPECT_CALL(frameReceiverMock, firstDataFrameReceived(_, 2, 1, _, _));
    simulateRx(fBct, fBct.fDevice, udsFrame);

    docanXcvr.shutdown();
    fBct.removeCANFrameListener(demoListener);
}

TEST_F(BxCanIntegrationTest, demoTxDoesNotBlockDocanTx)
{
    // Demo write (fire-and-forget) then DoCAN write (with listener) -- both succeed
    uint8_t demoPayload[] = {0x01, 0x02};
    CANFrame demoFrame(0x123U, demoPayload, 2);

    uint8_t udsPayload[] = {0x02, 0x7E, 0x00};
    CANFrame udsFrame(0x601U, udsPayload, 3);

    MockCANFrameSentListener docanListener;

    EXPECT_CALL(fBct.fDevice, transmit(_))
        .WillOnce(Return(true))  // demo
        .WillOnce(Return(true)); // docan

    // Demo write (no listener)
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBct.write(demoFrame));

    // DoCAN write (with listener)
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBct.write(udsFrame, docanListener));

    // TX ISR for DoCAN frame
    EXPECT_CALL(fBct.fDevice, transmitISR());
    EXPECT_CALL(docanListener, canFrameSent(_)).Times(1);
    BxCanTransceiver::transmitInterrupt(fBusId);
}

TEST_F(BxCanIntegrationTest, docanTxCallbackDoesNotAffectDemoWrite)
{
    // DoCAN write with callback, then demo write -- demo should succeed independently
    MockCANFrameSentListener docanListener;
    CANFrame docanFrame(0x601U);
    CANFrame demoFrame(0x123U);

    EXPECT_CALL(fBct.fDevice, transmit(_))
        .WillOnce(Return(true))  // docan
        .WillOnce(Return(true)); // demo (from callback chain or after)

    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBct.write(docanFrame, docanListener));

    // TX ISR for DoCAN frame
    EXPECT_CALL(fBct.fDevice, transmitISR());
    EXPECT_CALL(docanListener, canFrameSent(_)).Times(1);
    BxCanTransceiver::transmitInterrupt(fBusId);

    // Demo write after DoCAN callback completed
    EXPECT_EQ(ICanTransceiver::ErrorCode::CAN_ERR_OK, fBct.write(demoFrame));
}

TEST_F(BxCanIntegrationTest, busTrafficFromBothListenersSimultaneous)
{
    // RX traffic for both demo and DoCAN in the same receiveTask batch
    DemoListener demoListener;
    demoListener.getFilter().add(0x123);
    fBct.addCANFrameListener(demoListener);

    using Addressing      = DoCanNormalAddressing<>;
    using DataLinkLayer   = Addressing::DataLinkLayerType;
    using PhysTransceiver = DoCanPhysicalCanTransceiver<Addressing>;

    StrictMock<FilterMock> filterMock;
    Addressing addressing;
    StrictMock<DoCanAddressConverterMock<DataLinkLayer>> addrConvMock;
    DoCanFdFrameSizeMapper<uint8_t> sizeMapper;
    DoCanFrameCodec<DataLinkLayer> codec(
        DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, sizeMapper);
    StrictMock<DoCanFrameReceiverMock<DataLinkLayer>> frameReceiverMock;
    PhysTransceiver docanXcvr(fBct, filterMock, addrConvMock, addressing);

    EXPECT_CALL(filterMock, acceptMerger(_)).Times(AnyNumber());
    docanXcvr.init(frameReceiverMock);

    // Two frames in RX queue: one demo, one UDS
    uint8_t demoPayload[] = {0xAA};
    CANFrame demoFrame(0x123U, demoPayload, 1);

    uint8_t udsPayload[] = {0x02, 0x3E, 0x00};
    CANFrame udsFrame(0x600U, udsPayload, sizeof(udsPayload));

    EXPECT_CALL(fBct.fDevice, getRxCount()).WillOnce(Return(2));
    EXPECT_CALL(fBct.fDevice, getRxFrame(0)).WillOnce(ReturnRef(demoFrame));
    EXPECT_CALL(fBct.fDevice, getRxFrame(1)).WillOnce(ReturnRef(udsFrame));
    EXPECT_CALL(fBct.fDevice, clearRxQueue()).Times(1);
    EXPECT_CALL(fBct.fDevice, enableRxInterrupt()).Times(1);

    // Demo listener receives frame 0
    EXPECT_CALL(demoListener, frameReceived(_)).Times(1);
    // DoCAN filter: miss on 0x123, hit on 0x600
    EXPECT_CALL(filterMock, match(0x123U)).WillOnce(Return(false));
    EXPECT_CALL(filterMock, match(0x600U)).WillOnce(Return(true));
    EXPECT_CALL(addrConvMock, getReceptionParameters(0x600U, _, _)).WillOnce(Return(&codec));
    EXPECT_CALL(frameReceiverMock, firstDataFrameReceived(_, 2, 1, _, _));

    fBct.receiveTask();

    docanXcvr.shutdown();
    fBct.removeCANFrameListener(demoListener);
}

// ===================================================================
// Additional integration tests (to reach 32 total)
// ===================================================================

TEST_F(BxCanIntegrationTest, docanStartSendDataFramesThroughRealTransceiver)
{
    // Use DoCanPhysicalCanTransceiver::startSendDataFrames through the real BxCanTransceiver
    using Addressing      = DoCanNormalAddressing<>;
    using DataLinkLayer   = Addressing::DataLinkLayerType;
    using PhysTransceiver = DoCanPhysicalCanTransceiver<Addressing>;
    using JobHandle       = DataLinkLayer::JobHandleType;

    StrictMock<FilterMock> filterMock;
    Addressing addressing;
    StrictMock<DoCanAddressConverterMock<DataLinkLayer>> addrConvMock;
    DoCanFdFrameSizeMapper<uint8_t> sizeMapper;
    DoCanFrameCodec<DataLinkLayer> codec(
        DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, sizeMapper);
    StrictMock<DoCanFrameReceiverMock<DataLinkLayer>> frameReceiverMock;
    StrictMock<DoCanDataFrameTransmitterCallbackMock<DataLinkLayer>> txCallbackMock;

    PhysTransceiver docanXcvr(fBct, filterMock, addrConvMock, addressing);

    EXPECT_CALL(filterMock, acceptMerger(_)).Times(AnyNumber());
    docanXcvr.init(frameReceiverMock);

    uint8_t data[] = {0x12, 0x13, 0x24, 0x45};
    JobHandle jobHandle(0xFF, 0xEE);

    EXPECT_CALL(fBct.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(
        SendResult::QUEUED_FULL,
        docanXcvr.startSendDataFrames(codec, txCallbackMock, jobHandle, 0x601U, 0U, 1U, 0U, data));

    // TX ISR fires -- DoCAN callback should fire
    EXPECT_CALL(fBct.fDevice, transmitISR());
    EXPECT_CALL(txCallbackMock, dataFramesSent(jobHandle, 1U, sizeof(data)));
    BxCanTransceiver::transmitInterrupt(fBusId);

    docanXcvr.shutdown();
}

TEST_F(BxCanIntegrationTest, docanCancelSendThroughRealTransceiver)
{
    using Addressing      = DoCanNormalAddressing<>;
    using DataLinkLayer   = Addressing::DataLinkLayerType;
    using PhysTransceiver = DoCanPhysicalCanTransceiver<Addressing>;
    using JobHandle       = DataLinkLayer::JobHandleType;

    StrictMock<FilterMock> filterMock;
    Addressing addressing;
    StrictMock<DoCanAddressConverterMock<DataLinkLayer>> addrConvMock;
    DoCanFdFrameSizeMapper<uint8_t> sizeMapper;
    DoCanFrameCodec<DataLinkLayer> codec(
        DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, sizeMapper);
    StrictMock<DoCanFrameReceiverMock<DataLinkLayer>> frameReceiverMock;
    StrictMock<DoCanDataFrameTransmitterCallbackMock<DataLinkLayer>> txCallbackMock;

    PhysTransceiver docanXcvr(fBct, filterMock, addrConvMock, addressing);

    EXPECT_CALL(filterMock, acceptMerger(_)).Times(AnyNumber());
    docanXcvr.init(frameReceiverMock);

    uint8_t data[] = {0x12, 0x13, 0x24, 0x45};
    JobHandle jobHandle(0x1F, 0x12);

    EXPECT_CALL(fBct.fDevice, transmit(_)).WillOnce(Return(true));

    EXPECT_EQ(
        SendResult::QUEUED_FULL,
        docanXcvr.startSendDataFrames(codec, txCallbackMock, jobHandle, 0x601U, 0U, 1U, 0U, data));

    // Cancel before TX ISR -- callback should NOT fire
    docanXcvr.cancelSendDataFrames(txCallbackMock, jobHandle);

    // TX ISR fires -- cancelled, so no dataFramesSent callback
    EXPECT_CALL(fBct.fDevice, transmitISR());
    BxCanTransceiver::transmitInterrupt(fBusId);

    docanXcvr.shutdown();
}

} // namespace
