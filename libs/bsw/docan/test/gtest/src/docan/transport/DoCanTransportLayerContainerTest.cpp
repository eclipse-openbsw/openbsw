// Copyright 2024 Accenture.

#include "docan/transport/DoCanTransportLayerContainer.h"

#include "docan/addressing/DoCanAddressConverterMock.h"
#include "docan/common/DoCanConstants.h"
#include "docan/datalink/DoCanDataLinkLayer.h"
#include "docan/datalink/DoCanDefaultFrameSizeMapper.h"
#include "docan/datalink/DoCanFrameCodec.h"
#include "docan/datalink/DoCanFrameCodecConfigPresets.h"
#include "docan/datalink/DoCanPhysicalTransceiverMock.h"
#include "docan/transmitter/DoCanTickGeneratorMock.h"
#include "transport/BufferedTransportMessage.h"
#include "transport/TransportMessageProcessedListenerMock.h"
#include "transport/TransportMessageProvidingListenerMock.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <async/Types.h>
#include <bsp/timer/SystemTimerMock.h>

#include <gtest/esr_extensions.h>

namespace
{
using namespace docan;
using namespace testing;
using namespace transport;

using DataLinkLayerType = DoCanDataLinkLayer<uint32_t, uint16_t, uint8_t, 0U>;
using CodecType         = DoCanFrameCodec<DataLinkLayerType>;

struct DoCanTransportLayerContainerTest : ::testing::Test
{
    DoCanTransportLayerContainerTest() : _context(1)
    {
        EXPECT_CALL(_systemTimer, getSystemTimeNs()).WillRepeatedly(testing::Return(1000000UL));
        nowUs = 0;
    }

    MOCK_METHOD0(shutdownDone, void());

    uint32_t systemUs() { return nowUs; }

    StrictMock<DoCanAddressConverterMock<DataLinkLayerType>> _addressConverterMock;
    StrictMock<DoCanTickGeneratorMock> _tickGeneratorMock;
    StrictMock<DoCanPhysicalTransceiverMock<DataLinkLayerType>> _transceiverMock1;
    StrictMock<DoCanPhysicalTransceiverMock<DataLinkLayerType>> _transceiverMock2;
    StrictMock<TransportMessageProcessedListenerMock> _messageProcessedListenerMock;
    SystemTimerMock _systemTimer;
    DoCanParameters _parameters{
        ::estd::function<uint32_t()>::
            create<DoCanTransportLayerContainerTest, &DoCanTransportLayerContainerTest::systemUs>(
                *this),
        100U,
        200U,
        300U,
        400U,
        2U,
        3U,
        0U,
        0U};
    ::docan::declare::DoCanTransportLayerConfig<DataLinkLayerType, 2U, 3U, 64U> _config{
        _parameters};
    uint8_t _busId1;
    uint8_t _busId2;
    ::async::AsyncMock asyncMock;
    ::async::TestContext _context;
    uint32_t nowUs           = 0;
    uint8_t _loggerComponent = 8U;
};

TEST_F(DoCanTransportLayerContainerTest, testConstructedTransportLayers)
{
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    ::docan::declare::DoCanTransportLayerContainer<DataLinkLayerType, 2> cut;
    ::docan::declare::DoCanTransportLayerContainerBuilder<DataLinkLayerType> builder(
        cut, _addressConverterMock, _tickGeneratorMock, _config, _context, _loggerComponent);
    EXPECT_EQ(0U, cut.getTransportLayers().size());
    DoCanTransportLayer<DataLinkLayerType>& layer1
        = builder.addTransportLayer(_busId1, _transceiverMock1);
    EXPECT_EQ(1U, cut.getTransportLayers().size());
    EXPECT_EQ(&layer1, &cut.getTransportLayers()[0]);
    DoCanTransportLayer<DataLinkLayerType>& layer2
        = builder.addTransportLayer(_busId2, _transceiverMock2);
    EXPECT_EQ(2U, cut.getTransportLayers().size());
    EXPECT_EQ(&layer1, &cut.getTransportLayers()[0]);
    EXPECT_EQ(&layer2, &cut.getTransportLayers()[1]);
    {
        // initialize
        IDoCanFrameReceiver<DataLinkLayerType>* frameReceiver1 = nullptr;
        EXPECT_CALL(_transceiverMock1, init(_)).WillOnce(SaveRef<0>(&frameReceiver1));
        EXPECT_CALL(_transceiverMock2, init(_));
        cut.init();
        // send a segmented message needing tick
        uint8_t const data[]
            = {0x01,
               0x02,
               0x03,
               0x13,
               0x24,
               0x56,
               0x78,
               0x91,
               0xa1,
               0x94,
               0x10,
               0x19,
               0x88,
               0x77,
               0x66};
        BufferedTransportMessage<20> transportMessage;
        transportMessage.setSourceId(0x46U);
        transportMessage.setTargetId(0x89U);
        transportMessage.append(data, sizeof(data));
        transportMessage.setPayloadLength(sizeof(data));
        EXPECT_CALL(
            _addressConverterMock,
            getTransmissionParameters(DoCanTransportAddressPair(0x46U, 0x89U), _))
            .WillOnce(DoAll(
                SetArgReferee<1>(DoCanDataLinkAddressPair<uint32_t>(0x12345678U, 0x87654321U)),
                Return(&codec)));
        IDoCanDataFrameTransmitterCallback<DataLinkLayerType>* callback = nullptr;
        DataLinkLayerType::JobHandleType jobHandle(9, 8);
        EXPECT_CALL(_transceiverMock1, startSendDataFrames(_, _, _, 0x87654321U, 0U, 1U, 7U, _))
            .WillOnce(DoAll(
                WithArg<1>(SaveRef<0>(&callback)),
                SaveArg<2>(&jobHandle),
                Return(SendResult::QUEUED_FULL)));
        _context.handleExecute();
        EXPECT_EQ(
            AbstractTransportLayer::ErrorCode::TP_OK,
            cut.getTransportLayers()[0].send(transportMessage, &_messageProcessedListenerMock));
        _context.execute();
        Mock::VerifyAndClearExpectations(&_transceiverMock1);
        callback->dataFramesSent(jobHandle, 1U, 6U);
        EXPECT_CALL(
            _transceiverMock1,
            startSendDataFrames(_, Ref(*callback), jobHandle, 0x87654321U, 1U, 2U, 7U, _))
            .WillOnce(Return(SendResult::QUEUED_FULL));
        frameReceiver1->flowControlFrameReceived(0x12345678U, FlowStatus::CTS, 0x00, 0x1);
        Mock::VerifyAndClearExpectations(&_transceiverMock1);
        EXPECT_FALSE(cut.tick(nowUs));
        EXPECT_CALL(_tickGeneratorMock, tickNeeded());
        callback->dataFramesSent(jobHandle, 1U, 7U);
        // true because we just transitioned to sending consecutive frames, which requires
        // high-frequency ticks
        EXPECT_TRUE(cut.tick(nowUs));
        nowUs = 1000;
        EXPECT_CALL(
            _transceiverMock1,
            startSendDataFrames(_, Ref(*callback), jobHandle, 0x87654321U, 2U, 3U, 7U, _))
            .WillOnce(Return(SendResult::QUEUED_FULL));
        // Send single consecutive frame, thus no longer requiring high-frequency ticks
        EXPECT_FALSE(cut.tick(nowUs));
        EXPECT_CALL(
            _messageProcessedListenerMock,
            transportMessageProcessed(
                Ref(transportMessage),
                ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));
        callback->dataFramesSent(jobHandle, 1U, 2U);
    }
    {
        // cyclic tasks
        cut.cyclicTask(nowUs);
    }
    {
        // shutdown
        _context.handleExecute();
        cut.shutdown(DoCanTransportLayerContainer<DataLinkLayerType>::ShutdownCallbackType::create<
                     DoCanTransportLayerContainerTest,
                     &DoCanTransportLayerContainerTest::shutdownDone>(*this));
        EXPECT_CALL(_transceiverMock1, shutdown());
        EXPECT_CALL(_transceiverMock2, shutdown());
        EXPECT_CALL(*this, shutdownDone());
        _context.execute();
    }
}

} // anonymous namespace
