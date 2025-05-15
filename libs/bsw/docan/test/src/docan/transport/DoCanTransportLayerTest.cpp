// Copyright 2024 Accenture.

#include "docan/transport/DoCanTransportLayer.h"

#include "docan/addressing/DoCanAddressConverterMock.h"
#include "docan/common/DoCanConstants.h"
#include "docan/datalink/DoCanDataLinkLayer.h"
#include "docan/datalink/DoCanDefaultFrameSizeMapper.h"
#include "docan/datalink/DoCanFrameCodec.h"
#include "docan/datalink/DoCanFrameCodecConfigPresets.h"
#include "docan/datalink/DoCanPhysicalTransceiverMock.h"
#include "docan/datalink/IDoCanDataFrameTransmitter.h"
#include "docan/transmitter/DoCanTickGeneratorMock.h"
#include "docan/transport/DoCanTransportLayerConfig.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <bsp/timer/SystemTimerMock.h>
#include <etl/delegate.h>
#include <etl/memory.h>
#include <etl/span.h>
#include <transport/BufferedTransportMessage.h>
#include <transport/TransportMessageProcessedListenerMock.h>
#include <transport/TransportMessageProvidingListenerMock.h>
#include <util/logger/ComponentMappingMock.h>
#include <util/logger/LoggerOutputMock.h>

#include <gtest/esr_extensions.h>

namespace
{
using namespace docan;
using namespace ::util::logger;
using namespace transport;
using namespace testing;

using DataLinkLayerType       = DoCanDataLinkLayer<uint32_t, uint16_t, uint8_t, 0U>;
using CodecType               = DoCanFrameCodec<DataLinkLayerType>;
using ConnectionType          = DoCanConnection<DataLinkLayerType>;
using DataLinkAddressPairType = DataLinkLayerType::AddressPairType;

struct DoCanTransportLayerTest : ::testing::Test
{
    DoCanTransportLayerTest()
    : _parameters(
        ::etl::delegate<
            uint32_t()>::create<DoCanTransportLayerTest, &DoCanTransportLayerTest::systemUs>(*this),
        200,
        3543,
        1232,
        3442,
        98,
        153,
        0,
        0)
    , _messageProvidingListenerMock(false)
    , _config(_parameters)
    , _context(1)
    , nowUs(0)
    , _loggerComponent(8)
    {
        EXPECT_CALL(_systemTimer, getSystemTimeNs()).WillRepeatedly(testing::Return(1000000UL));
        Logger::init(_componentMappingMock, _loggerOutputMock);
        nowUs = 0;
    }

    ~DoCanTransportLayerTest() override { Logger::shutdown(); }

    MOCK_METHOD(void, shutdownDone, (AbstractTransportLayer&));

    void expectLog(Level level);
    void expectLog(Level level, uint32_t address);

    uint32_t systemUs() { return nowUs; }

    StrictMock<ComponentMappingMock> _componentMappingMock;
    StrictMock<LoggerOutputMock> _loggerOutputMock;

    uint8_t _busId = 0xFF;
    DoCanParameters _parameters;
    StrictMock<DoCanAddressConverterMock<DataLinkLayerType>> _addressConverterMock;
    StrictMock<DoCanPhysicalTransceiverMock<DataLinkLayerType>> _transceiverMock;
    StrictMock<DoCanTickGeneratorMock> _tickGeneratorMock;
    StrictMock<TransportMessageProvidingListenerMock> _messageProvidingListenerMock;
    StrictMock<TransportMessageProcessedListenerMock> _messageProcessedListenerMock;
    ::testing::SystemTimerMock _systemTimer;
    ::docan::declare::DoCanTransportLayerConfig<DataLinkLayerType, 2U, 2U, 8U> _config;
    ::async::TestContext _context;
    ::async::AsyncMock asyncMock;
    uint32_t nowUs;
    uint8_t _loggerComponent;
};

TEST_F(DoCanTransportLayerTest, testTransportMessageReceptionLifecycle)
{
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    DoCanTransportLayer<DataLinkLayerType> cut(
        _busId,
        _context,
        _addressConverterMock,
        _transceiverMock,
        _tickGeneratorMock,
        _config,
        _loggerComponent);
    cut.setTransportMessageListener(&_messageProvidingListenerMock);
    cut.setTransportMessageProvider(&_messageProvidingListenerMock);
    IDoCanFrameReceiver<DataLinkLayerType>* frameReceiver = nullptr;
    EXPECT_CALL(_transceiverMock, init(_)).WillOnce(SaveRef<0>(&frameReceiver));
    EXPECT_EQ(AbstractTransportLayer::ErrorCode::TP_OK, cut.init());
    Mock::VerifyAndClearExpectations(&_transceiverMock);

    // receive single frame message without immediate allocation success
    {
        BufferedTransportMessage<4> transportMessage;
        uint8_t const data[] = {0x12, 0x34, 0x56, 0x78};
        ConnectionType connection(
            codec,
            DataLinkAddressPairType(0x1234857U, 0x7654321U),
            DoCanTransportAddressPair(0x35, 0x69));
        EXPECT_CALL(
            _messageProvidingListenerMock, getTransportMessage(_busId, 0x35, 0x69, 4U, _, _))
            .WillOnce(Return(ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE));
        frameReceiver->firstDataFrameReceived(connection, 4U, 1U, 0U, data);
        Mock::VerifyAndClearExpectations(&_addressConverterMock);
        Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);

        EXPECT_CALL(
            _messageProvidingListenerMock, getTransportMessage(_busId, 0x35, 0x69, 4U, _, _))
            .WillOnce(DoAll(
                SetArgReferee<5>(&transportMessage),
                Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
        ITransportMessageProcessedListener* notificationListener = nullptr;
        EXPECT_CALL(
            _messageProvidingListenerMock, messageReceived(_busId, Ref(transportMessage), _))
            .WillOnce(DoAll(
                SaveArg<2>(&notificationListener),
                Return(ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)));
        cut.cyclicTask(nowUs);
        nowUs += 200000;
        cut.cyclicTask(nowUs);
        Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
        ASSERT_TRUE(notificationListener != nullptr);
        // check message
        EXPECT_EQ(0x35U, transportMessage.getSourceId());
        EXPECT_EQ(0x69U, transportMessage.getTargetId());
        EXPECT_TRUE(::etl::equal(
            etl::span<uint8_t const>(data),
            etl::span<uint8_t const>(
                transportMessage.getPayload(), transportMessage.getPayloadLength())));
        // release
        EXPECT_CALL(_messageProvidingListenerMock, releaseTransportMessage(Ref(transportMessage)));
        _context.handleExecute();
        notificationListener->transportMessageProcessed(
            transportMessage,
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
        _context.execute();
    }
    // receive segmented message
    {
        BufferedTransportMessage<7> transportMessage;
        uint8_t const data[] = {0x12, 0x34, 0x56, 0x78, 0x91, 0x73, 0x85};
        ConnectionType connection(
            codec,
            DataLinkAddressPairType(0x13348447U, 0x76333211U),
            DoCanTransportAddressPair(0x37, 0x96));
        EXPECT_CALL(
            _messageProvidingListenerMock, getTransportMessage(_busId, 0x37, 0x96, 7U, _, _))
            .WillOnce(DoAll(
                SetArgReferee<5>(&transportMessage),
                Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
        EXPECT_CALL(
            _transceiverMock,
            sendFlowControl(
                _,
                connection.getDataLinkAddressPair().getTransmissionAddress(),
                FlowStatus::CTS,
                0x00,
                0x00))
            .WillOnce(Return(true));
        frameReceiver->firstDataFrameReceived(
            connection, 7U, 2U, 7U, ::etl::span<uint8_t const>(data).first(6U));
        Mock::VerifyAndClearExpectations(&_addressConverterMock);
        Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);

        ITransportMessageProcessedListener* notificationListener = nullptr;
        EXPECT_CALL(
            _messageProvidingListenerMock, messageReceived(_busId, Ref(transportMessage), _))
            .WillOnce(DoAll(
                SaveArg<2>(&notificationListener),
                Return(ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)));
        frameReceiver->consecutiveDataFrameReceived(
            0x13348447U, 1U, ::etl::span<uint8_t const>(data).subspan(6U));
        // check message
        ASSERT_TRUE(notificationListener != nullptr);
        EXPECT_EQ(0x37U, transportMessage.getSourceId());
        EXPECT_EQ(0x96U, transportMessage.getTargetId());
        EXPECT_TRUE(::etl::equal(
            etl::span<uint8_t const>(data),
            etl::span<uint8_t const>(
                transportMessage.getPayload(), transportMessage.getPayloadLength())));
        // release
        EXPECT_CALL(_messageProvidingListenerMock, releaseTransportMessage(Ref(transportMessage)));
        _context.handleExecute();
        notificationListener->transportMessageProcessed(
            transportMessage,
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
        _context.execute();
    }
    // receive segmented message with escape sequence
    {
        constexpr auto MESSAGE_SIZE = 4999U;
        constexpr auto FRAME_SIZE   = 7U;
        constexpr auto FRAMES       = (MESSAGE_SIZE / FRAME_SIZE) + 1;
        BufferedTransportMessage<MESSAGE_SIZE> transportMessage;
        uint8_t data[MESSAGE_SIZE];
        for (size_t i = 0; i < MESSAGE_SIZE; i++)
        {
            data[i] = i & 0xFF;
        }
        auto slice = ::etl::span<uint8_t>(data);
        ConnectionType connection(
            codec,
            DataLinkAddressPairType(0x13348447U, 0x76333211U),
            DoCanTransportAddressPair(0x37, 0x96));
        EXPECT_CALL(
            _messageProvidingListenerMock, getTransportMessage(_busId, 0x37, 0x96, 4999U, _, _))
            .WillOnce(DoAll(
                SetArgReferee<5>(&transportMessage),
                Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
        EXPECT_CALL(
            _transceiverMock,
            sendFlowControl(
                _,
                connection.getDataLinkAddressPair().getTransmissionAddress(),
                FlowStatus::CTS,
                0x00,
                0x00))
            .WillOnce(Return(true));
        frameReceiver->firstDataFrameReceived(
            connection, MESSAGE_SIZE, FRAMES, FRAME_SIZE, slice.first(FRAME_SIZE));
        slice.advance(FRAME_SIZE);
        Mock::VerifyAndClearExpectations(&_addressConverterMock);
        Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);

        ITransportMessageProcessedListener* notificationListener = nullptr;
        EXPECT_CALL(
            _messageProvidingListenerMock, messageReceived(_busId, Ref(transportMessage), _))
            .WillOnce(DoAll(
                SaveArg<2>(&notificationListener),
                Return(ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)));
        for (size_t i = 1; i < FRAMES - 1; i++)
        {
            frameReceiver->consecutiveDataFrameReceived(
                0x13348447U, i & 0x0F, slice.first(FRAME_SIZE));
            slice.advance(FRAME_SIZE);
        }
        // Last frame.
        EXPECT_LT(slice.size(), FRAME_SIZE);
        frameReceiver->consecutiveDataFrameReceived(
            0x13348447U, (FRAMES - 1) & 0x0F, slice.first(slice.size()));
        slice.advance(slice.size());

        // check message
        EXPECT_EQ(0x37U, transportMessage.getSourceId());
        EXPECT_EQ(0x96U, transportMessage.getTargetId());
        EXPECT_THAT(
            ::etl::span<uint8_t const>(
                transportMessage.getPayload(), transportMessage.getPayloadLength()),
            ElementsAreArray(data));
        // release
        EXPECT_CALL(_messageProvidingListenerMock, releaseTransportMessage(Ref(transportMessage)));
        _context.handleExecute();
        notificationListener->transportMessageProcessed(
            transportMessage,
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
        _context.execute();
    }
    // receive unexpected consecutive frame
    {
        expectLog(LEVEL_WARN, 0x1234857U);
        uint8_t const data[] = {0x12, 0x34, 0x56, 0x78};
        frameReceiver->consecutiveDataFrameReceived(0x1234857U, 7U, data);
    }

    _context.handleExecute();
    EXPECT_FALSE(cut.shutdown(
        AbstractTransportLayer::ShutdownDelegate::
            create<DoCanTransportLayerTest, &DoCanTransportLayerTest::shutdownDone>(*this)));
    EXPECT_CALL(_transceiverMock, shutdown());
    EXPECT_CALL(*this, shutdownDone(_));
    _context.execute();
}

TEST_F(DoCanTransportLayerTest, testTransportMessageTransmissionLifecycle)
{
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    DoCanTransportLayer<DataLinkLayerType> cut(
        _busId,
        _context,
        _addressConverterMock,
        _transceiverMock,
        _tickGeneratorMock,
        _config,
        _loggerComponent);
    cut.setTransportMessageListener(&_messageProvidingListenerMock);
    cut.setTransportMessageProvider(&_messageProvidingListenerMock);
    IDoCanFrameReceiver<DataLinkLayerType>* frameReceiver = nullptr;
    EXPECT_CALL(_transceiverMock, init(_)).WillOnce(SaveRef<0>(&frameReceiver));
    EXPECT_EQ(AbstractTransportLayer::ErrorCode::TP_OK, cut.init());
    Mock::VerifyAndClearExpectations(&_transceiverMock);

    // send a segmented message
    {
        BufferedTransportMessage<30> transportMessage;
        uint8_t const data[]
            = {0x12, 0x34, 0x56, 0x78, 0x19, 0x38, 0x9a, 0x5f, 0x14, 0x91, 0xa3, 0x57, 0x89, 0x99};
        transportMessage.setSourceId(0x56U);
        transportMessage.setTargetId(0x64U);
        transportMessage.append(data, sizeof(data));
        transportMessage.setPayloadLength(sizeof(data));
        EXPECT_CALL(
            _addressConverterMock,
            getTransmissionParameters(DoCanTransportAddressPair(0x56U, 0x64U), _))
            .WillOnce(DoAll(
                SetArgReferee<1>(DataLinkAddressPairType(0x1235689U, 0x986321U)), Return(&codec)));
        _context.handleExecute();
        EXPECT_EQ(
            AbstractTransportLayer::ErrorCode::TP_OK,
            cut.send(transportMessage, &_messageProcessedListenerMock));

        IDoCanDataFrameTransmitterCallback<DataLinkLayerType>* callback = nullptr;
        DataLinkLayerType::JobHandleType jobHandle(7, 6);
        EXPECT_CALL(
            _transceiverMock,
            startSendDataFrames(_, _, _, 0x986321U, 0U, 1U, 7U, ElementsAreArray(data)))
            .WillOnce(DoAll(
                WithArg<1>(SaveRef<0>(&callback)),
                SaveArg<2>(&jobHandle),
                Return(SendResult::QUEUED_FULL)));
        _context.execute();

        Mock::VerifyAndClearExpectations(&_addressConverterMock);
        Mock::VerifyAndClearExpectations(&_transceiverMock);
        callback->dataFramesSent(jobHandle, 1U, 6U);
        // expect flow control
        auto slice0{::etl::span<uint8_t const>(data).subspan(6U)};
        EXPECT_CALL(
            _transceiverMock,
            startSendDataFrames(
                _,
                Ref(*callback),
                jobHandle,
                0x986321U,
                1U,
                2U,
                7U,
                ElementsAreArray(slice0.data(), slice0.size())))
            .WillOnce(Return(SendResult::QUEUED_FULL));
        frameReceiver->flowControlFrameReceived(0x1235689U, FlowStatus::CTS, 0U, 1U);
        Mock::VerifyAndClearExpectations(&_transceiverMock);
        EXPECT_CALL(_tickGeneratorMock, tickNeeded());
        callback->dataFramesSent(jobHandle, 1U, 7U);
        Mock::VerifyAndClearExpectations(&_tickGeneratorMock);
        // tick received
        auto slice1{::etl::span<uint8_t const>(data).subspan(13U)};
        EXPECT_CALL(
            _transceiverMock,
            startSendDataFrames(
                _,
                Ref(*callback),
                jobHandle,
                0x986321U,
                2U,
                3U,
                7U,
                ElementsAreArray(slice1.data(), slice1.size())))
            .WillOnce(Return(SendResult::QUEUED_FULL));
        nowUs = 1000;
        cut.tick(nowUs);
        Mock::VerifyAndClearExpectations(&_transceiverMock);
        EXPECT_CALL(
            _messageProcessedListenerMock,
            transportMessageProcessed(
                Ref(transportMessage),
                ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));
        callback->dataFramesSent(jobHandle, 1U, 1U);
    }
    {
        // send to unknown address pair
        BufferedTransportMessage<10> transportMessage;
        uint8_t const data[] = {0x12, 0x34, 0x56, 0x78, 0x19};
        transportMessage.setSourceId(0x56U);
        transportMessage.setTargetId(0x64U);
        transportMessage.append(data, sizeof(data));
        transportMessage.setPayloadLength(sizeof(data));
        EXPECT_CALL(
            _addressConverterMock,
            getTransmissionParameters(DoCanTransportAddressPair(0x56U, 0x64U), _))
            .WillOnce(Return(nullptr));
        expectLog(LEVEL_WARN);
        EXPECT_EQ(
            AbstractTransportLayer::ErrorCode::TP_SEND_FAIL,
            cut.send(transportMessage, &_messageProcessedListenerMock));
    }
    {
        // receive unexpected flow control frame
        expectLog(LEVEL_WARN, 0x1235689U);
        frameReceiver->flowControlFrameReceived(0x1235689U, FlowStatus::CTS, 0U, 1U);
    }

    _context.handleExecute();
    EXPECT_FALSE(cut.shutdown(
        AbstractTransportLayer::ShutdownDelegate::
            create<DoCanTransportLayerTest, &DoCanTransportLayerTest::shutdownDone>(*this)));
    EXPECT_CALL(_transceiverMock, shutdown());
    EXPECT_CALL(*this, shutdownDone(_));
    _context.execute();
}

void DoCanTransportLayerTest::expectLog(Level const level)
{
    EXPECT_CALL(_componentMappingMock, isEnabled(_loggerComponent, level)).WillOnce(Return(false));
}

void DoCanTransportLayerTest::expectLog(Level const level, uint32_t address)
{
    expectLog(level);
    EXPECT_CALL(_addressConverterMock, formatDataLinkAddress(address, _)).WillOnce(Return("abc"));
}

} // anonymous namespace
