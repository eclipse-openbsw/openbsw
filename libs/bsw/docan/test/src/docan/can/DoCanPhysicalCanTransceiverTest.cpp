// Copyright 2024 Accenture.

#include "docan/can/DoCanPhysicalCanTransceiver.h"

#include "docan/addressing/DoCanAddressConverterMock.h"
#include "docan/addressing/DoCanNormalAddressing.h"
#include "docan/common/DoCanConstants.h"
#include "docan/datalink/DoCanDataFrameTransmitterCallbackMock.h"
#include "docan/datalink/DoCanDefaultFrameSizeMapper.h"
#include "docan/datalink/DoCanFdFrameSizeMapper.h"
#include "docan/datalink/DoCanFrameCodec.h"
#include "docan/datalink/DoCanFrameCodecConfigPresets.h"
#include "docan/datalink/DoCanFrameReceiverMock.h"

#include <can/filter/FilterMock.h>
#include <can/filter/IMerger.h>
#include <can/transceiver/ICanTransceiverMock.h>

#include <gtest/esr_extensions.h>
#include <gtest/gtest.h>

namespace
{
using namespace can;
using namespace docan;
using namespace testing;

template<typename AddressingType>
struct DoCanPhysicalCanTransceiverTest : public ::testing::Test
{
    using CodecType         = AddressingType;
    using DataLinkLayerType = typename AddressingType::DataLinkLayerType;
    using JobHandle         = typename CodecType::DataLinkLayerType::JobHandleType;

    DoCanPhysicalCanTransceiverTest(
        DoCanFrameCodecConfig<typename DoCanFrameCodec<DataLinkLayerType>::FrameSizeType> const&
            config)
    : _codec(config, _sizeMapper)
    {}

    StrictMock<ICanTransceiverMock> _canTransceiverMock;
    StrictMock<FilterMock> _filterMock;
    DoCanFdFrameSizeMapper<uint8_t> const _sizeMapper;
    AddressingType _addressing;
    DoCanFrameCodec<DataLinkLayerType> _codec;
    StrictMock<DoCanAddressConverterMock<typename AddressingType::DataLinkLayerType>>
        _addressConverterMock;
    StrictMock<DoCanFrameReceiverMock<typename CodecType::DataLinkLayerType>> _frameReceiverMock;
    StrictMock<DoCanDataFrameTransmitterCallbackMock<typename CodecType::DataLinkLayerType>>
        _frameTransmitterCallbackMock;
};

struct TestWithNormalAddressing : public DoCanPhysicalCanTransceiverTest<DoCanNormalAddressing<>>
{
    TestWithNormalAddressing()
    : DoCanPhysicalCanTransceiverTest<DoCanNormalAddressing<>>(
        DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC)
    {}
};

TEST_F(TestWithNormalAddressing, testInitializeAndShutdown)

{
    DoCanPhysicalCanTransceiver<TestWithNormalAddressing::CodecType> cut(
        _canTransceiverMock, _filterMock, _addressConverterMock, _addressing);
    ICANFrameListener* canFrameListener = 0L;
    EXPECT_CALL(_canTransceiverMock, addCANFrameListener(_))
        .WillOnce(SaveRef<0>(&canFrameListener));
    cut.init(_frameReceiverMock);
    Mock::VerifyAndClearExpectations(&_canTransceiverMock);
    EXPECT_EQ(&_filterMock, &canFrameListener->getFilter());

    EXPECT_CALL(_canTransceiverMock, removeCANFrameListener(Ref(*canFrameListener)));
    cut.shutdown();
}

TEST_F(TestWithNormalAddressing, testTransceiverSendDataFrames)
{
    DoCanPhysicalCanTransceiver<TestWithNormalAddressing::CodecType> cut(
        _canTransceiverMock, _filterMock, _addressConverterMock, _addressing);
    {
        uint8_t const data[]            = {0x12U, 0x13U, 0x24U, 0x45U};
        uint8_t const expectedPayload[] = {0x04U, 0x12U, 0x13U, 0x24U, 0x45U};
        CANFrame expectedFrame(0x1234897U, expectedPayload, sizeof(expectedPayload));
        ICANFrameSentListener* sentListener = 0L;
        EXPECT_CALL(_canTransceiverMock, write(expectedFrame, _))
            .WillOnce(DoAll(
                WithArg<1>(SaveRef<0>(&sentListener)),
                Return(ICanTransceiver::ErrorCode::CAN_ERR_OK)));
        JobHandle jobHandle(0xff, 0xee);
        EXPECT_EQ(
            SendResult::QUEUED_FULL,
            cut.startSendDataFrames(
                _codec, _frameTransmitterCallbackMock, jobHandle, 0x1234897U, 0U, 1U, 0U, data));
        Mock::VerifyAndClearExpectations(&_canTransceiverMock);
        // expect callback when CAN frame has been sent
        EXPECT_CALL(_frameTransmitterCallbackMock, dataFramesSent(jobHandle, 1U, sizeof(data)));
        sentListener->canFrameSent(expectedFrame);
    }
    {
        // accept next frame
        uint8_t const data[]            = {0x13U, 0x24U, 0x45U, 0x89, 0x91, 0x82, 0x71, 0x91};
        uint8_t const expectedPayload[] = {0x10U, 0x08, 0x13U, 0x24U, 0x45U, 0x89, 0x91, 0x82};
        CANFrame expectedFrame(0x1234897U, expectedPayload, sizeof(expectedPayload));
        ICANFrameSentListener* sentListener = 0L;
        EXPECT_CALL(_canTransceiverMock, write(expectedFrame, _))
            .WillOnce(DoAll(
                WithArg<1>(SaveRef<0>(&sentListener)),
                Return(ICanTransceiver::ErrorCode::CAN_ERR_OK)));
        JobHandle jobHandle(0xaf, 0xea);
        EXPECT_EQ(
            SendResult::QUEUED_FULL,
            cut.startSendDataFrames(
                _codec, _frameTransmitterCallbackMock, jobHandle, 0x1234897U, 0U, 2U, 7U, data));
        Mock::VerifyAndClearExpectations(&_canTransceiverMock);

        // expect error code when sending again
        EXPECT_EQ(
            SendResult::FULL,
            cut.startSendDataFrames(
                _codec, _frameTransmitterCallbackMock, jobHandle, 0x1234897U, 0U, 2U, 7U, data));

        // expect callback when CAN frame has been sent
        EXPECT_CALL(_frameTransmitterCallbackMock, dataFramesSent(jobHandle, 1U, 6U));
        sentListener->canFrameSent(expectedFrame);
    }
    {
        // Expect error in case of invalid coding
        uint8_t const data[] = {0x13U, 0x24U, 0x45U, 0x89, 0x91, 0x82, 0x71, 0x91};
        JobHandle jobHandle(0xaf, 0xe1);
        EXPECT_EQ(
            SendResult::INVALID,
            cut.startSendDataFrames(
                _codec, _frameTransmitterCallbackMock, jobHandle, 0x1234897U, 0U, 2U, 0U, data));
    }
    {
        // Expect error in case of CAN driver returning queue full
        uint8_t const data[] = {0x13U, 0x24U};
        EXPECT_CALL(_canTransceiverMock, write(_, _))
            .WillOnce(Return(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL));
        JobHandle jobHandle(0xaf, 0xea);
        EXPECT_EQ(
            SendResult::FULL,
            cut.startSendDataFrames(
                _codec, _frameTransmitterCallbackMock, jobHandle, 0x1234897U, 0U, 1U, 0U, data));
    }
    {
        // Expect error in case of CAN driver returning other error
        uint8_t const data[] = {0x13U, 0x24U};
        EXPECT_CALL(_canTransceiverMock, write(_, _))
            .WillOnce(Return(ICanTransceiver::ErrorCode::CAN_ERR_TX_FAIL));
        JobHandle jobHandle(0xaf, 0xea);
        EXPECT_EQ(
            SendResult::FAILED,
            cut.startSendDataFrames(
                _codec, _frameTransmitterCallbackMock, jobHandle, 0x1234897U, 0U, 1U, 0U, data));
    }
    {
        // Cancel send frame and expect no callback
        uint8_t const data[]                = {0x12U, 0x13U, 0x24U, 0x45U};
        ICANFrameSentListener* sentListener = 0L;
        EXPECT_CALL(_canTransceiverMock, write(_, _))
            .WillOnce(DoAll(
                WithArg<1>(SaveRef<0>(&sentListener)),
                Return(ICanTransceiver::ErrorCode::CAN_ERR_OK)));
        JobHandle jobHandle(0x1f, 0x12);
        EXPECT_EQ(
            SendResult::QUEUED_FULL,
            cut.startSendDataFrames(
                _codec, _frameTransmitterCallbackMock, jobHandle, 0x1234897U, 0U, 1U, 0U, data));
        Mock::VerifyAndClearExpectations(&_canTransceiverMock);
        cut.cancelSendDataFrames(_frameTransmitterCallbackMock, jobHandle);
        sentListener->canFrameSent(CANFrame());
        Mock::VerifyAndClearExpectations(&_canTransceiverMock);
    }
    {
        // Cancel send frame and expect to be able to send something else
        uint8_t const data[]                = {0x12U, 0x13U, 0x24U, 0x45U};
        ICANFrameSentListener* sentListener = 0L;
        EXPECT_CALL(_canTransceiverMock, write(_, _))
            .WillOnce(DoAll(
                WithArg<1>(SaveRef<0>(&sentListener)),
                Return(ICanTransceiver::ErrorCode::CAN_ERR_OK)));
        JobHandle jobHandle(0x1f, 0x12);
        EXPECT_EQ(
            SendResult::QUEUED_FULL,
            cut.startSendDataFrames(
                _codec, _frameTransmitterCallbackMock, jobHandle, 0x1234897U, 0U, 1U, 0U, data));
        Mock::VerifyAndClearExpectations(&_canTransceiverMock);
        // TX timeout from Transmitter
        cut.cancelSendDataFrames(_frameTransmitterCallbackMock, jobHandle);

        // Different data size to ensure dataFramesSent is sending this, not the previous message.
        uint8_t const data2[]                = {0x91, 0x82, 0x71};
        ICANFrameSentListener* sentListener2 = 0L;
        EXPECT_CALL(_canTransceiverMock, write(_, _))
            .WillOnce(DoAll(
                WithArg<1>(SaveRef<0>(&sentListener2)),
                Return(ICanTransceiver::ErrorCode::CAN_ERR_OK)));
        // Even if they are the same job handle, even though they shouldnt be.
        JobHandle jobHandle2(jobHandle);
        EXPECT_EQ(
            SendResult::QUEUED_FULL,
            cut.startSendDataFrames(
                _codec, _frameTransmitterCallbackMock, jobHandle2, 0x1234897U, 0U, 1U, 0U, data2));
        Mock::VerifyAndClearExpectations(&_canTransceiverMock);
        EXPECT_CALL(_frameTransmitterCallbackMock, dataFramesSent(jobHandle2, 1U, sizeof(data2)));

        sentListener->canFrameSent(CANFrame());
        Mock::VerifyAndClearExpectations(&_canTransceiverMock);
    }
    {
        // Nothing to do if no pending send
        cut.cancelSendDataFrames(_frameTransmitterCallbackMock, JobHandle(6, 7));
        Mock::VerifyAndClearExpectations(&_canTransceiverMock);
    }
    {
        // Only the current sender can cancel sending frames
        uint8_t const data[]                = {0x12U, 0x13U, 0x24U, 0x45U};
        ICANFrameSentListener* sentListener = 0L;
        EXPECT_CALL(_canTransceiverMock, write(_, _))
            .WillOnce(DoAll(
                WithArg<1>(SaveRef<0>(&sentListener)),
                Return(ICanTransceiver::ErrorCode::CAN_ERR_OK)));
        JobHandle jobHandle(0xff, 0xaa);
        EXPECT_EQ(
            SendResult::QUEUED_FULL,
            cut.startSendDataFrames(
                _codec, _frameTransmitterCallbackMock, jobHandle, 0x1234897U, 0U, 1U, 0U, data));
        Mock::VerifyAndClearExpectations(&_canTransceiverMock);
        cut.cancelSendDataFrames(_frameTransmitterCallbackMock, JobHandle(4, 3));
        DoCanDataFrameTransmitterCallbackMock<CodecType::DataLinkLayerType>
            frameTransmitterCallbackMock2;
        cut.cancelSendDataFrames(frameTransmitterCallbackMock2, jobHandle);
        // expect anyway response
        EXPECT_CALL(_frameTransmitterCallbackMock, dataFramesSent(jobHandle, 1U, sizeof(data)));
        sentListener->canFrameSent(CANFrame());
        Mock::VerifyAndClearExpectations(&_canTransceiverMock);
        // ignore an unexpected frame sent response
        sentListener->canFrameSent(CANFrame());
        Mock::VerifyAndClearExpectations(&_canTransceiverMock);
    }
}

TEST_F(TestWithNormalAddressing, testTransceiverSendDataFramesWithEscapeSequence)
{
    DoCanPhysicalCanTransceiver<TestWithNormalAddressing::CodecType> cut(
        _canTransceiverMock, _filterMock, _addressConverterMock, _addressing);
    {
        // First frame with EscSeq
        uint8_t data[0x1357];
        for (size_t i = 0; i < 0x1357; i++)
        {
            data[i] = (i + 0xA5) & 0xFF;
        }
        uint8_t const expectedPayload[] = {0x10U, 0x00, 0x00U, 0x00U, 0x13U, 0x57U, 0xA5, 0xA6};
        CANFrame expectedFrame(0x1234897U, expectedPayload, sizeof(expectedPayload));
        ICANFrameSentListener* sentListener = 0L;
        EXPECT_CALL(_canTransceiverMock, write(expectedFrame, _))
            .WillOnce(DoAll(
                WithArg<1>(SaveRef<0>(&sentListener)),
                Return(ICanTransceiver::ErrorCode::CAN_ERR_OK)));
        JobHandle jobHandle(0xaf, 0xea);
        EXPECT_EQ(
            SendResult::QUEUED_FULL,
            cut.startSendDataFrames(
                _codec, _frameTransmitterCallbackMock, jobHandle, 0x1234897U, 0U, 708U, 7U, data));
        Mock::VerifyAndClearExpectations(&_canTransceiverMock);

        // expect error code when sending again
        EXPECT_EQ(
            SendResult::FULL,
            cut.startSendDataFrames(
                _codec, _frameTransmitterCallbackMock, jobHandle, 0x1234897U, 0U, 708U, 7U, data));

        // expect callback when CAN frame has been sent
        EXPECT_CALL(_frameTransmitterCallbackMock, dataFramesSent(jobHandle, 1U, 2U));
        sentListener->canFrameSent(expectedFrame);
    }
}

TEST_F(TestWithNormalAddressing, testTransceiverSendFlowControlFrame)
{
    DoCanPhysicalCanTransceiver<TestWithNormalAddressing::CodecType> cut(
        _canTransceiverMock, _filterMock, _addressConverterMock, _addressing);
    {
        uint8_t const expectedPayload[] = {0x32U, 0x14, 0x56};
        CANFrame expectedFrame(0x12345897U, expectedPayload, sizeof(expectedPayload));
        EXPECT_CALL(_canTransceiverMock, write(expectedFrame))
            .WillOnce(Return(ICanTransceiver::ErrorCode::CAN_ERR_OK));
        EXPECT_TRUE(cut.sendFlowControl(_codec, 0x12345897U, FlowStatus::OVFLW, 0x14, 0x56));
    }
    {
        // return send failed in case transceiver cannot send
        EXPECT_CALL(_canTransceiverMock, write(_))
            .WillOnce(Return(ICanTransceiver::ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL));
        EXPECT_FALSE(cut.sendFlowControl(_codec, 0x12345897U, FlowStatus::OVFLW, 0x14, 0x56));
    }
}

TEST_F(TestWithNormalAddressing, testTransceiverReceivesFrame)
{
    DoCanPhysicalCanTransceiver<TestWithNormalAddressing::CodecType> cut(
        _canTransceiverMock, _filterMock, _addressConverterMock, _addressing);
    ICANFrameListener* canFrameListener = 0L;
    EXPECT_CALL(_canTransceiverMock, addCANFrameListener(_))
        .WillOnce(SaveRef<0>(&canFrameListener));
    cut.init(_frameReceiverMock);
    Mock::VerifyAndClearExpectations(&_canTransceiverMock);
    {
        uint8_t const payload[] = {0x32, 0x56, 0x78};
        EXPECT_CALL(
            _frameReceiverMock,
            flowControlFrameReceived(0x1237846U, FlowStatus::OVFLW, 0x56, 0x78));
        EXPECT_CALL(_addressConverterMock, getReceptionParameters(0x1237846U, _, _))
            .WillOnce(Return(&_codec));
        canFrameListener->frameReceived(CANFrame(0x1237846U, payload, sizeof(payload)));
        Mock::VerifyAndClearExpectations(&_frameReceiverMock);
    }
    EXPECT_CALL(_canTransceiverMock, removeCANFrameListener(Ref(*canFrameListener)));
    cut.shutdown();
    Mock::VerifyAndClearExpectations(&_canTransceiverMock);
    {
        // expect no frame forwarded after shutdown
        uint8_t const payload[] = {0x34, 0x56, 0x78};
        canFrameListener->frameReceived(CANFrame(0x1237846U, payload, sizeof(payload)));
    }
}

TEST_F(TestWithNormalAddressing, testTransceiverReceivesFrameWithEscapeSequence)
{
    DoCanPhysicalCanTransceiver<TestWithNormalAddressing::CodecType> cut(
        _canTransceiverMock, _filterMock, _addressConverterMock, _addressing);
    ICANFrameListener* canFrameListener = 0L;
    EXPECT_CALL(_canTransceiverMock, addCANFrameListener(_))
        .WillOnce(SaveRef<0>(&canFrameListener));
    cut.init(_frameReceiverMock);
    Mock::VerifyAndClearExpectations(&_canTransceiverMock);
    {
        uint8_t const payload[] = {0x10, 0x00, 0x00, 0x00, 0x10, 0x01, 0xA5, 0xB4};
        EXPECT_CALL(_frameReceiverMock, firstDataFrameReceived(_, 0x1001, _, _, _));
        EXPECT_CALL(_addressConverterMock, getReceptionParameters(0x1237846U, _, _))
            .WillOnce(Return(&_codec));
        canFrameListener->frameReceived(CANFrame(0x1237846U, payload, sizeof(payload)));
        Mock::VerifyAndClearExpectations(&_frameReceiverMock);
    }
    EXPECT_CALL(_canTransceiverMock, removeCANFrameListener(Ref(*canFrameListener)));
    cut.shutdown();
    Mock::VerifyAndClearExpectations(&_canTransceiverMock);
    {
        // expect no frame forwarded after shutdown
        uint8_t const payload[] = {0x10, 0x00, 0x00, 0x00, 0x10, 0x01, 0xA5, 0xB4};
        canFrameListener->frameReceived(CANFrame(0x1237846U, payload, sizeof(payload)));
    }
}

TEST_F(TestWithNormalAddressing, testTransceiverReceivesFrameToNonexistentAddress)
{
    DoCanPhysicalCanTransceiver<TestWithNormalAddressing::CodecType> cut(
        _canTransceiverMock, _filterMock, _addressConverterMock, _addressing);
    ICANFrameListener* canFrameListener = 0L;
    EXPECT_CALL(_canTransceiverMock, addCANFrameListener(_))
        .WillOnce(SaveRef<0>(&canFrameListener));
    cut.init(_frameReceiverMock);
    Mock::VerifyAndClearExpectations(&_canTransceiverMock);
    uint8_t const payload[] = {0x32, 0x56, 0x78};
    {
        EXPECT_CALL(
            _frameReceiverMock, flowControlFrameReceived(0x1237846U, FlowStatus::OVFLW, 0x56, 0x78))
            .Times(0);
        EXPECT_CALL(_addressConverterMock, getReceptionParameters(0x1237846U, _, _))
            .WillOnce(Return(nullptr));
        canFrameListener->frameReceived(CANFrame(0x1237846U, payload, sizeof(payload)));
        Mock::VerifyAndClearExpectations(&_frameReceiverMock);
    }
    EXPECT_CALL(_canTransceiverMock, removeCANFrameListener(Ref(*canFrameListener)));
    cut.shutdown();
    Mock::VerifyAndClearExpectations(&_canTransceiverMock);
    {
        // expect no frame forwarded after shutdown
        EXPECT_CALL(_addressConverterMock, getReceptionParameters(0x1237846U, _, _)).Times(0);
        canFrameListener->frameReceived(CANFrame(0x1237846U, payload, sizeof(payload)));
    }
}
} // anonymous namespace
