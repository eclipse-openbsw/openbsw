// Copyright 2024 Accenture.

#include "docan/receiver/DoCanReceiver.h"

#include "docan/addressing/DoCanAddressConverterMock.h"
#include "docan/common/DoCanConstants.h"
#include "docan/datalink/DoCanDataLinkAddressPair.h"
#include "docan/datalink/DoCanDefaultFrameSizeMapper.h"
#include "docan/datalink/DoCanFlowControlFrameTransmitterMock.h"
#include "docan/datalink/DoCanFrameCodec.h"
#include "docan/datalink/DoCanFrameCodecConfigPresets.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <bsp/timer/SystemTimerMock.h>
#include <transport/BufferedTransportMessage.h>
#include <transport/TransportMessageProvidingListenerMock.h>
#include <util/logger/ComponentMappingMock.h>
#include <util/logger/LoggerOutputMock.h>

#include <estd/functional.h>

#include <limits>

namespace
{
using namespace docan;
using namespace ::util::logger;
using namespace testing;
using namespace transport;

struct DataLinkLayer
{
    static uint32_t const INVALID_ADDRESS = 0xffffffffU;

    using AddressType     = uint32_t;
    using AddressPairType = DoCanDataLinkAddressPair<uint32_t>;
    using MessageSizeType = uint16_t;
    using FrameIndexType  = uint16_t;
    using FrameSizeType   = uint8_t;
    using JobHandleType   = uint16_t;
};

using CodecType = DoCanFrameCodec<DataLinkLayer>;

struct DoCanReceiverTest : ::testing::Test
{
    DoCanReceiverTest() : _context(1), nowUs(0), _loggerComponent(8)
    {
        EXPECT_CALL(_systemTimer, getSystemTimeNs()).WillRepeatedly(testing::Return(1000000UL));
        Logger::init(_componentMappingMock, _loggerOutputMock);
    }

    ~DoCanReceiverTest() { Logger::shutdown(); }

    void expectLog(Level level);
    void expectLog(Level level, uint32_t address);
    void expectLog(Level level, uint32_t address, char const* logMessage);

    uint32_t systemUs() { return nowUs; }

    StrictMock<ComponentMappingMock> _componentMappingMock;
    StrictMock<LoggerOutputMock> _loggerOutputMock;
    StrictMock<TransportMessageProvidingListenerMock> _messageProvidingListenerMock{false};
    StrictMock<DoCanFlowControlFrameTransmitterMock<DataLinkLayer>>
        _flowControlFrameTransmitterMock;
    StrictMock<DoCanAddressConverterMock<DataLinkLayer>> _addressConverterMock;
    ::transport::BufferedTransportMessage<100> _transportMessage1;
    ::transport::BufferedTransportMessage<100> _transportMessage2;
    uint8_t _busId = 0xFF;

    static uint16_t const waitAllocateTimeout    = 100U;
    static uint16_t const waitRxTimeout          = 200U;
    static uint16_t const waitTxCallbackTimeout  = 300U;
    static uint16_t const waitFlowControlTimeout = 400U;
    static uint8_t const maxAllocateRetryCount   = 2U;
    static uint8_t const maxFlowControlWaitCount = 3U;
    static uint16_t const minSeparationTime      = 0U;
    static uint8_t const maxBlockSize            = 0U;
    DoCanParameters _parameters{
        ::estd::function<uint32_t()>::create<DoCanReceiverTest, &DoCanReceiverTest::systemUs>(
            *this),
        waitAllocateTimeout,
        waitRxTimeout,
        waitTxCallbackTimeout,
        waitFlowControlTimeout,
        maxAllocateRetryCount,
        maxFlowControlWaitCount,
        minSeparationTime,
        maxBlockSize};
    ::testing::SystemTimerMock _systemTimer;

    ::async::TestContext _context;
    ::async::AsyncMock asyncMock;
    uint32_t nowUs;
    uint8_t _loggerComponent;
};

TEST_F(DoCanReceiverTest, testAssertsOnTooSmallBlocksInBlockPool)
{
    ::util::estd::declare::block_pool<1U, sizeof(void*)> messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ASSERT_THROW(cut.init(), ::estd::assert_exception);
}

TEST_F(DoCanReceiverTest, testAssertsOnTooBigBlocksInBlockPool)
{
    ::util::estd::declare::block_pool<
        1U,
        static_cast<size_t>(::std::numeric_limits<DataLinkLayer::FrameSizeType>::max()) + 1U
            + sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, 7U>)>
        messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ASSERT_THROW(cut.init(), ::estd::assert_exception);
}

/**
 * \req: [BSW_DoCAN_187], [BSW_DoCAN_157]
 */
TEST_F(DoCanReceiverTest, testReceiveSingleFrameMessageAndShutdown)
{
    ::util::estd::declare::
        block_pool<5U, sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, 7U>)>
            messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t const data[] = {0xab, 0xcd, 0xef, 0x19, 0x28};
    ::estd::slice<uint8_t const> peek;
    EXPECT_CALL(_messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, 5U, _, _))
        .WillOnce(DoAll(
            SaveArg<4>(&peek),
            SetArgReferee<5>(&_transportMessage1),
            Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
    ITransportMessageProcessedListener* notificationListener = 0L;
    EXPECT_CALL(
        _messageProvidingListenerMock, messageReceived(_busId, Ref(_transportMessage1), NotNull()))
        .WillOnce(DoAll(
            SaveArg<2>(&notificationListener),
            Return(ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)));
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    DoCanConnection<DataLinkLayer> connection(
        codec,
        DataLinkLayer::AddressPairType(0x1234, 0x5678),
        DoCanTransportAddressPair(0x14, 0x23));
    cut.firstDataFrameReceived(connection, 5U, 1U, 7U, data);
    ASSERT_EQ(5U, peek.size());
    EXPECT_TRUE(::estd::memory::is_equal(data, peek));
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
    Mock::VerifyAndClearExpectations(&_flowControlFrameTransmitterMock);
    EXPECT_EQ(
        0,
        ::estd::memory::compare(
            data,
            ::estd::slice<uint8_t const>::from_pointer(
                _transportMessage1.getPayload(), _transportMessage1.getPayloadLength())));
    EXPECT_EQ(0x14U, _transportMessage1.getSourceId());
    EXPECT_EQ(0x23U, _transportMessage1.getTargetId());
    // expect release on processed
    EXPECT_CALL(_messageProvidingListenerMock, releaseTransportMessage(Ref(_transportMessage1)));
    _context.handleExecute();
    notificationListener->transportMessageProcessed(
        _transportMessage1,
        ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    _context.execute();

    // shutdown
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

TEST_F(DoCanReceiverTest, testReceiveShortSingleFrameMessageAndShutdown)
{
    ::util::estd::declare::
        block_pool<5U, sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, 7U>)>
            messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t const data[] = {0xab, 0xcd, 0xef};
    ::estd::slice<uint8_t const> peek;
    EXPECT_CALL(_messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, 3U, _, _))
        .WillOnce(DoAll(
            SaveArg<4>(&peek),
            SetArgReferee<5>(&_transportMessage1),
            Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
    ITransportMessageProcessedListener* notificationListener = 0L;
    EXPECT_CALL(
        _messageProvidingListenerMock, messageReceived(_busId, Ref(_transportMessage1), NotNull()))
        .WillOnce(DoAll(
            SaveArg<2>(&notificationListener),
            Return(ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)));
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    DoCanConnection<DataLinkLayer> connection(
        codec,
        DataLinkLayer::AddressPairType(0x1234, 0x5678),
        DoCanTransportAddressPair(0x14, 0x23));
    cut.firstDataFrameReceived(connection, 3U, 1U, 0, data);
    ASSERT_EQ(3U, peek.size());
    EXPECT_TRUE(::estd::memory::is_equal(data, peek));
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
    Mock::VerifyAndClearExpectations(&_flowControlFrameTransmitterMock);
    EXPECT_EQ(
        0,
        ::estd::memory::compare(
            data,
            ::estd::slice<uint8_t const>::from_pointer(
                _transportMessage1.getPayload(), _transportMessage1.getPayloadLength())));
    EXPECT_EQ(0x14U, _transportMessage1.getSourceId());
    EXPECT_EQ(0x23U, _transportMessage1.getTargetId());
    // expect release on processed
    EXPECT_CALL(_messageProvidingListenerMock, releaseTransportMessage(Ref(_transportMessage1)));
    _context.handleExecute();
    notificationListener->transportMessageProcessed(
        _transportMessage1,
        ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    _context.execute();

    // shutdown
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

/**
 * \req: [BSW_DoCAN_187]
 */
TEST_F(DoCanReceiverTest, testReceiveSecondSingleFrameMessage)
{
    ::util::estd::declare::
        block_pool<5U, sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, 7U>)>
            messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t const data1[] = {0xab, 0xcd, 0xef, 0x19, 0x28};
    ::estd::slice<uint8_t const> peek1;
    // receive the single frame
    EXPECT_CALL(_messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, 5U, _, _))
        .WillOnce(
            DoAll(SaveArg<4>(&peek1), Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x1234, 0x5678),
            DoCanTransportAddressPair(0x14, 0x23)),
        5U,
        1U,
        7U,
        data1);
    ASSERT_EQ(5U, peek1.size());
    EXPECT_TRUE(::estd::memory::is_equal(data1, peek1));
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
    // receive second single frame
    uint8_t const data2[] = {0xab, 0xdc, 0xfd, 0x91, 0x28, 0x31};
    ::estd::slice<uint8_t const> peek2;
    EXPECT_CALL(_messageProvidingListenerMock, getTransportMessage(_busId, 0x17, 0x43, 6U, _, _))
        .WillOnce(DoAll(
            SaveArg<4>(&peek2),
            SetArgReferee<5>(&_transportMessage2),
            Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
    ITransportMessageProcessedListener* notificationListener = 0L;
    EXPECT_CALL(
        _messageProvidingListenerMock, messageReceived(_busId, Ref(_transportMessage2), NotNull()))
        .WillOnce(DoAll(
            SaveArg<2>(&notificationListener),
            Return(ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)));
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x5443, 0x8876),
            DoCanTransportAddressPair(0x17, 0x43)),
        6U,
        1U,
        7U,
        data2);
    ASSERT_EQ(6U, peek2.size());
    EXPECT_TRUE(::estd::memory::is_equal(data2, peek2));
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
    Mock::VerifyAndClearExpectations(&_flowControlFrameTransmitterMock);
    EXPECT_EQ(
        0,
        ::estd::memory::compare(
            data2,
            ::estd::slice<uint8_t const>::from_pointer(
                _transportMessage2.getPayload(), _transportMessage2.getPayloadLength())));
    EXPECT_EQ(0x17U, _transportMessage2.getSourceId());
    EXPECT_EQ(0x43U, _transportMessage2.getTargetId());
    // reallocate first
    EXPECT_CALL(_messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, 5U, _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&_transportMessage1),
            Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
    EXPECT_CALL(
        _messageProvidingListenerMock, messageReceived(_busId, Ref(_transportMessage1), NotNull()))
        .WillOnce(DoAll(
            SaveArg<2>(&notificationListener),
            Return(ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)));
    EXPECT_CALL(_messageProvidingListenerMock, releaseTransportMessage(Ref(_transportMessage2)));
    _context.handleExecute();
    notificationListener->transportMessageProcessed(
        _transportMessage2,
        ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    _context.execute();

    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
    Mock::VerifyAndClearExpectations(&_flowControlFrameTransmitterMock);
    EXPECT_EQ(
        0,
        ::estd::memory::compare(
            data1,
            ::estd::slice<uint8_t const>::from_pointer(
                _transportMessage1.getPayload(), _transportMessage1.getPayloadLength())));
    EXPECT_EQ(0x14U, _transportMessage1.getSourceId());
    EXPECT_EQ(0x23U, _transportMessage1.getTargetId());

    // shutdown
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

TEST_F(DoCanReceiverTest, testReceiveSingleFrameMessageWithReceptionError)
{
    ::util::estd::declare::
        block_pool<5U, sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, 7U>)>
            messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t data[] = {0xab, 0xcd, 0xef, 0x19, 0x28};
    EXPECT_CALL(_messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, 5U, _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&_transportMessage1),
            Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
    EXPECT_CALL(
        _messageProvidingListenerMock, messageReceived(_busId, Ref(_transportMessage1), NotNull()))
        .WillOnce(Return(ITransportMessageListener::ReceiveResult::RECEIVED_ERROR));
    EXPECT_CALL(_messageProvidingListenerMock, releaseTransportMessage(Ref(_transportMessage1)));
    expectLog(LEVEL_WARN, 0x1234);
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x1234, 0x5678),
            DoCanTransportAddressPair(0x14, 0x23)),
        5U,
        1U,
        7U,
        data);
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
    Mock::VerifyAndClearExpectations(&_flowControlFrameTransmitterMock);
    EXPECT_EQ(
        0,
        ::estd::memory::compare(
            data,
            ::estd::slice<uint8_t const>::from_pointer(
                _transportMessage1.getPayload(), _transportMessage1.getPayloadLength())));
    EXPECT_EQ(0x14U, _transportMessage1.getSourceId());
    EXPECT_EQ(0x23U, _transportMessage1.getTargetId());

    // shutdown
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

/**
 * \req: [BSW_DoCAN_187]
 */
TEST_F(
    DoCanReceiverTest, testReceiveSecondSingleFrameMessageWithLimitedTransportMessagesAndShutdown)
{
    ::util::estd::declare::
        block_pool<5U, sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, 7U>)>
            messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t data1[] = {0xab, 0xcd, 0xef, 0x19, 0x28};
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, sizeof(data1), _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&_transportMessage1),
            Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
    ITransportMessageProcessedListener* notificationListener = 0L;
    EXPECT_CALL(
        _messageProvidingListenerMock, messageReceived(_busId, Ref(_transportMessage1), NotNull()))
        .WillOnce(DoAll(
            SaveArg<2>(&notificationListener),
            Return(ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)));
    // receive the single frame
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x1234, 0x5678),
            DoCanTransportAddressPair(0x14, 0x23)),
        sizeof(data1),
        1U,
        7U,
        data1);
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
    // receive the second single frame
    uint8_t data2[] = {0xab, 0xcd, 0xef, 0x19, 0x28, 0x31};
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x17, 0x43, sizeof(data2), _, _))
        .WillOnce(Return(ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE));
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x5443, 0x8876),
            DoCanTransportAddressPair(0x17, 0x43)),
        sizeof(data2),
        1U,
        7U,
        data2);
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);

    // don't expect a new received message
    nowUs = 99999U;
    cut.cyclicTask(nowUs);
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x17, 0x43, sizeof(data2), _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&_transportMessage1),
            Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
    EXPECT_CALL(
        _messageProvidingListenerMock, messageReceived(_busId, Ref(_transportMessage1), NotNull()))
        .WillOnce(Return(ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR));
    // now expect the frame to be received
    nowUs += 1U;
    cut.cyclicTask(nowUs);

    // shutdown
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

/**
 * \req: [BSW_DoCAN_187]
 */
TEST_F(DoCanReceiverTest, testReceiveSecondSingleFrameMessageWithoutFreeMessageReceiverAndShutdown)
{
    ::util::estd::declare::
        block_pool<1U, sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, 7U>)>
            messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t const data1[] = {0xab, 0xcd, 0xef, 0x19, 0x28};
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, sizeof(data1), _, _))
        .WillOnce(Return(ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE));
    // receive the single frame
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x1234, 0x5678),
            DoCanTransportAddressPair(0x14, 0x23)),
        sizeof(data1),
        1U,
        7U,
        data1);
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
    // second message
    uint8_t data2[] = {0xab, 0xcd, 0xef, 0x19, 0x28, 0x31};
    // receive the single frame
    expectLog(LEVEL_WARN, 0x5443);
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x5443, 0x8876),
            DoCanTransportAddressPair(0x17, 0x43)),
        sizeof(data2),
        1U,
        7U,
        data2);

    // shutdown
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

/**
 * \req: [BSW_DoCAN_187], [BSW_DoCAN_74], [BSW_DoCAN_158]
 */
TEST_F(DoCanReceiverTest, testReceiveSegmentedMessageAndShutdown)
{
    ::util::estd::declare::
        block_pool<5U, sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, 7U>)>
            messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t data[] = {
        0xab, 0xcd, 0xef, 0x19, 0x28, 0x37, 0x46, 0x55, 0x64, 0x73, 0x82, 0x91, 0x11, 0x22, 0x33};
    // receive the first frame
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, sizeof(data), _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&_transportMessage1),
            Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
    EXPECT_CALL(
        _flowControlFrameTransmitterMock, sendFlowControl(_, 0x5678, FlowStatus::CTS, 0U, 0U))
        .WillOnce(Return(true));
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x1234, 0x5678),
            DoCanTransportAddressPair(0x14, 0x23)),
        sizeof(data),
        3U,
        7U,
        ::estd::slice<uint8_t const>::from_pointer(data, 6U));
    // receive consecutive frame of an unknown receiver
    expectLog(LEVEL_WARN, 0x4455);
    cut.consecutiveDataFrameReceived(
        0x4455, 0x1U, ::estd::slice<uint8_t const>::from_pointer(data + 6U, 7U));
    // receive too short consecutive frame
    expectLog(LEVEL_WARN, 0x1234);
    cut.consecutiveDataFrameReceived(
        0x1234, 0x1U, ::estd::slice<uint8_t const>::from_pointer(data + 6U, 2U));
    // receive valid consecutive frames
    cut.consecutiveDataFrameReceived(
        0x1234, 0x1U, ::estd::slice<uint8_t const>::from_pointer(data + 6U, 7U));
    EXPECT_CALL(
        _messageProvidingListenerMock, messageReceived(_busId, Ref(_transportMessage1), NotNull()))
        .WillOnce(Return(ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR));
    cut.consecutiveDataFrameReceived(
        0x1234, 0x2U, ::estd::slice<uint8_t const>::from_pointer(data + 13U, 2U));
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
    EXPECT_EQ(
        0,
        ::estd::memory::compare(
            data,
            ::estd::slice<uint8_t const>::from_pointer(
                _transportMessage1.getPayload(), _transportMessage1.getPayloadLength())));
    EXPECT_EQ(0x14U, _transportMessage1.getSourceId());
    EXPECT_EQ(0x23U, _transportMessage1.getTargetId());

    // shutdown
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

// Some timeout will show that the expiration and receiving right before works even when passing
// uint wrap around.
TEST_F(
    DoCanReceiverTest,
    testReceiveMultipleConsecutiveFrameMessagesWithSomeRxTimeoutsAtTimeWraparound)
{
    // 3 messages before wrap around (timeout at time max, max-1 and max-2), rest after.
    nowUs = ::std::numeric_limits<uint32_t>::max() - (waitRxTimeout * 1000) - 2;

    ASSERT_GT(nowUs, ::std::numeric_limits<int32_t>::max());
    constexpr uint8_t MESSAGE_SIZE   = 12; // Should be only 2 frames
    constexpr uint8_t NO_OF_MESSAGES = 12;
    ::util::estd::declare::block_pool<
        NO_OF_MESSAGES,
        sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, MESSAGE_SIZE>)>
        messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);

    uint8_t data[NO_OF_MESSAGES][MESSAGE_SIZE];
    ::transport::BufferedTransportMessage<MESSAGE_SIZE> transportMessage[NO_OF_MESSAGES];
    for (uint8_t i = 0; i < NO_OF_MESSAGES; i++)
    {
        for (uint8_t j = 0; j < MESSAGE_SIZE; j++)
        {
            data[i][j] = i + j;
        }
    }
    for (uint8_t i = 0; i < NO_OF_MESSAGES; i++)
    {
        // receive the first frame
        EXPECT_CALL(
            _messageProvidingListenerMock,
            getTransportMessage(_busId, 0x14 + i, 0x23 + i, MESSAGE_SIZE, _, _))
            .WillOnce(DoAll(
                SetArgReferee<5>(&transportMessage[i]),
                Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
        EXPECT_CALL(
            _flowControlFrameTransmitterMock,
            sendFlowControl(_, 0x5678 + i, FlowStatus::CTS, 0U, 0U))
            .WillOnce(Return(true));
        cut.firstDataFrameReceived(
            DoCanConnection<DataLinkLayer>(
                codec,
                DataLinkLayer::AddressPairType(0x1234 + i, 0x5678 + i),
                DoCanTransportAddressPair(0x14 + i, 0x23 + i)),
            MESSAGE_SIZE,
            2U,
            7U,
            ::estd::slice<uint8_t const>::from_pointer(data[i], 6U));

        nowUs++;
        cut.cyclicTask(nowUs);
    }
    while (nowUs < ::std::numeric_limits<uint32_t>::max() - 124U)
    {
        nowUs += 123;
        cut.cyclicTask(nowUs);
    }
    // 3 Microsec before first timeout
    nowUs = ::std::numeric_limits<uint32_t>::max() - 3;
    cut.cyclicTask(nowUs);

    for (uint8_t i = 0; i < NO_OF_MESSAGES; i++)
    {
        if (i % 3 == 0)
        {
            // Leave to timeout
            expectLog(LEVEL_WARN, 0x1234 + i);
            EXPECT_CALL(
                _messageProvidingListenerMock, releaseTransportMessage(Ref(transportMessage[i])));
        }
        else
        {
            // Receive
            EXPECT_CALL(
                _messageProvidingListenerMock,
                messageReceived(_busId, Ref(transportMessage[i]), NotNull()))
                .WillOnce(Return(ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR));
            // receive valid consecutive frames
            cut.consecutiveDataFrameReceived(
                0x1234 + i, 0x1U, ::estd::slice<uint8_t const>::from_pointer(data[i] + 6U, 6U));
        }

        nowUs++;
        cut.cyclicTask(nowUs);
    }

    // Assert that we wrapped around.
    ASSERT_LT(nowUs, ::std::numeric_limits<int32_t>::max());
    nowUs++;
    cut.cyclicTask(nowUs);

    // shutdown
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

// The messages are first initialized, 2 with successful allocation, 2 without, with 1ms between
// each init.
// 1 & 2. With the cyclicTask they should be sorted, so the first 2 messages (second and third) go
// into Allocation timeout, retries the allocation and then allowed to succeed. These 2 will then be
// sorted to be last, as they now have a new RX timeout.
// 3 & 4. As time passes the first and RX timeout, the messages that was allocated at init (first
// and third) will fail.
// 5 & 6. Time will pass to allocationTimeout + RX timeout, and the last 2 remaining messages will
// fail.
TEST_F(DoCanReceiverTest, testMultipleDifferentTimeoutsWithDifferentExpiry)
{
    nowUs                              = 0;
    constexpr uint8_t MESSAGE_SIZE     = 12;
    constexpr uint8_t NO_OF_MESSAGES   = 4;
    //   2 RX timeouts, 2 Allocate timeout
    constexpr uint8_t RX_TIMEOUT       = 0; // Even
    constexpr uint8_t ALLOCATE_TIMEOUT = 1; // Odd

    ::util::estd::declare::block_pool<
        NO_OF_MESSAGES,
        sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, MESSAGE_SIZE>)>
        messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);

    uint8_t data[NO_OF_MESSAGES][MESSAGE_SIZE];
    ::transport::BufferedTransportMessage<MESSAGE_SIZE> transportMessage[NO_OF_MESSAGES];
    constexpr uint32_t timeSteps = 1000;
    for (uint8_t i = 0; i < NO_OF_MESSAGES; i++)
    {
        nowUs = i * timeSteps;
        cut.cyclicTask(nowUs);
        for (uint8_t j = 0; j < MESSAGE_SIZE; j++)
        {
            data[i][j] = i + j;
        }
        // For allocation time outs, receive but do not allocate
        if ((i % 2) == ALLOCATE_TIMEOUT)
        {
            EXPECT_CALL(
                _messageProvidingListenerMock,
                getTransportMessage(_busId, 0x14 + i, 0x23 + i, MESSAGE_SIZE, _, _))
                .WillOnce(Return(ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE));
            EXPECT_CALL(
                _flowControlFrameTransmitterMock,
                sendFlowControl(_, 0x5678 + i, FlowStatus::WAIT, 0U, 0U))
                .WillOnce(Return(true));
        }
        else
        {
            EXPECT_CALL(
                _messageProvidingListenerMock,
                getTransportMessage(_busId, 0x14 + i, 0x23 + i, MESSAGE_SIZE, _, _))
                .WillOnce(DoAll(
                    SetArgReferee<5>(&transportMessage[i]),
                    Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
            EXPECT_CALL(
                _flowControlFrameTransmitterMock,
                sendFlowControl(_, 0x5678 + i, FlowStatus::CTS, 0U, 0U))
                .WillOnce(Return(true));
        }
        cut.firstDataFrameReceived(
            DoCanConnection<DataLinkLayer>(
                codec,
                DataLinkLayer::AddressPairType(0x1234 + i, 0x5678 + i),
                DoCanTransportAddressPair(0x14 + i, 0x23 + i)),
            MESSAGE_SIZE,
            2U,
            7U,
            ::estd::slice<uint8_t const>::from_pointer(data[i], 6U));
    }

    Mock::VerifyAndClearExpectations(&_flowControlFrameTransmitterMock);
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);

    // 1. First Allocation timeout
    // First nothing
    nowUs = waitAllocateTimeout * 1000 - 1;
    cut.cyclicTask(nowUs);

    // Then fail and try to reallocate, allow to succeed this time
    EXPECT_CALL(
        _messageProvidingListenerMock,
        getTransportMessage(
            _busId, 0x14 + ALLOCATE_TIMEOUT, 0x23 + ALLOCATE_TIMEOUT, MESSAGE_SIZE, _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&transportMessage[ALLOCATE_TIMEOUT]),
            Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
    EXPECT_CALL(
        _flowControlFrameTransmitterMock,
        sendFlowControl(_, 0x5678 + ALLOCATE_TIMEOUT, FlowStatus::CTS, 0U, 0U))
        .WillOnce(Return(true));
    nowUs += (ALLOCATE_TIMEOUT * timeSteps) + 1;
    // Save what time this will go into RX Timeout
    auto const lastRxTimeouts = nowUs + (waitRxTimeout * 1000);
    cut.cyclicTask(nowUs);

    // 2. Second Allocation timeout (Index ALLOCATE_TIMEOUT + 2), 2 timesteps inbetween
    // First nothing
    nowUs += timeSteps;
    cut.cyclicTask(nowUs);

    // Then fail and try to reallocate, allow to succeed this time
    EXPECT_CALL(
        _messageProvidingListenerMock,
        getTransportMessage(
            _busId, 0x14 + ALLOCATE_TIMEOUT + 2, 0x23 + ALLOCATE_TIMEOUT + 2, MESSAGE_SIZE, _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&transportMessage[ALLOCATE_TIMEOUT + 2]),
            Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
    EXPECT_CALL(
        _flowControlFrameTransmitterMock,
        sendFlowControl(_, 0x5678 + ALLOCATE_TIMEOUT + 2, FlowStatus::CTS, 0U, 0U))
        .WillOnce(Return(true));
    nowUs += timeSteps;
    cut.cyclicTask(nowUs);

    // 3. First RX Timeout
    // First nothing.
    nowUs = waitRxTimeout * 1000 - 1;
    cut.cyclicTask(nowUs);
    // Then fail
    expectLog(LEVEL_WARN, 0x1234 + RX_TIMEOUT);
    EXPECT_CALL(
        _messageProvidingListenerMock, releaseTransportMessage(Ref(transportMessage[RX_TIMEOUT])));
    nowUs += (RX_TIMEOUT * timeSteps) + 1;
    cut.cyclicTask(nowUs);

    // 4. Second RX Timeout
    // First nothing.
    nowUs += timeSteps;
    cut.cyclicTask(nowUs);
    // Then fail
    expectLog(LEVEL_WARN, 0x1234 + RX_TIMEOUT + 2);
    EXPECT_CALL(
        _messageProvidingListenerMock,
        releaseTransportMessage(Ref(transportMessage[RX_TIMEOUT + 2])));
    nowUs += timeSteps;
    cut.cyclicTask(nowUs);

    // Previously delayed allocation timeouts
    // 5. First
    // First nothing.
    nowUs = lastRxTimeouts - 1;
    cut.cyclicTask(nowUs);
    // Then fail
    expectLog(LEVEL_WARN, 0x1234 + ALLOCATE_TIMEOUT);
    EXPECT_CALL(
        _messageProvidingListenerMock,
        releaseTransportMessage(Ref(transportMessage[ALLOCATE_TIMEOUT])));
    nowUs += 1;
    cut.cyclicTask(nowUs);

    // 6. Second
    // First nothing.
    nowUs += timeSteps;
    cut.cyclicTask(nowUs);
    // Then fail
    expectLog(LEVEL_WARN, 0x1234 + ALLOCATE_TIMEOUT + 2);
    EXPECT_CALL(
        _messageProvidingListenerMock,
        releaseTransportMessage(Ref(transportMessage[ALLOCATE_TIMEOUT + 2])));
    nowUs += timeSteps;
    cut.cyclicTask(nowUs);

    // shutdown
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

TEST_F(DoCanReceiverTest, testReceiveSegmentedMessageWithWrongSequenceNumberAndShutdown)
{
    ::util::estd::declare::
        block_pool<5U, sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, 7U>)>
            messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t data[] = {0xab, 0xcd, 0xef, 0x19, 0x28, 0x37, 0x46, 0x55, 0x64, 0x73,
                      0x82, 0x91, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    // receive the first frame
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, sizeof(data), _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&_transportMessage1),
            Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
    EXPECT_CALL(
        _flowControlFrameTransmitterMock, sendFlowControl(_, 0x5678, FlowStatus::CTS, 0U, 0U))
        .WillOnce(Return(true));
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x1234, 0x5678),
            DoCanTransportAddressPair(0x14, 0x23)),
        sizeof(data),
        3U,
        7U,
        ::estd::slice<uint8_t const>::from_pointer(data, 6U));

    // receive invalid consecutive frames, index 2 instead of 1
    // To test bad sequence number, second index frame needs to be the same size as is expected from
    // first index frame.
    EXPECT_CALL(_messageProvidingListenerMock, releaseTransportMessage(Ref(_transportMessage1)));
    expectLog(
        LEVEL_WARN, 0x1234, "DoCanReceiver(%s)::%s(%s): Frame with bad sequence number received");
    cut.consecutiveDataFrameReceived(
        0x1234, 0x2U, ::estd::slice<uint8_t const>::from_pointer(data + 13U, 7U));
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
    // shutdown
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

/**
 * \req: [BSW_DoCAN_187]
 */
TEST_F(DoCanReceiverTest, testReceiveFirstConsecutiveFrameOfSegmentedMessageDuringAllocation)
{
    ::util::estd::declare::
        block_pool<5U, sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, 7U>)>
            messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t data[] = {
        0xab, 0xcd, 0xef, 0x19, 0x28, 0x37, 0x46, 0x55, 0x64, 0x73, 0x82, 0x91, 0x11, 0x22, 0x33};
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, sizeof(data), _, _))
        .WillOnce(Return(ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE));
    EXPECT_CALL(
        _flowControlFrameTransmitterMock, sendFlowControl(_, 0x5678, FlowStatus::WAIT, 0U, 0U))
        .WillOnce(Return(true));
    // receive the first frame
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x1234, 0x5678),
            DoCanTransportAddressPair(0x14, 0x23)),
        sizeof(data),
        3U,
        7U,
        ::estd::slice<uint8_t const>::from_pointer(data, 6U));
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
    // and immediately the consecutive one which isn't expected
    expectLog(LEVEL_WARN, 0x1234);
    cut.consecutiveDataFrameReceived(
        0x1234U, 0x1U, ::estd::slice<uint8_t const>::from_pointer(data + 6U, 7U));

    // shutdown
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

/**
 * \req: [BSW_DoCAN_187], [BSW_DoCAN_163]
 */
TEST_F(
    DoCanReceiverTest,
    testReceiveSegmentedMessageAndSingleFrameMessagesWithLimitedResourcesAndShutdown)
{
    ::util::estd::declare::
        block_pool<5U, sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, 7U>)>
            messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t data1[] = {
        0xab, 0xcd, 0xef, 0x19, 0x28, 0x37, 0x46, 0x55, 0x64, 0x73, 0x82, 0x91, 0x11, 0x22, 0x33};
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, sizeof(data1), _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&_transportMessage1),
            Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
    EXPECT_CALL(
        _flowControlFrameTransmitterMock, sendFlowControl(_, 0x5678, FlowStatus::CTS, 0x0U, 0x0U))
        .WillOnce(Return(true));
    // receive the first frame
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x1234, 0x5678),
            DoCanTransportAddressPair(0x14, 0x23)),
        sizeof(data1),
        3U,
        7U,
        ::estd::slice<uint8_t const>::from_pointer(data1, 6U));
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
    Mock::VerifyAndClearExpectations(&_flowControlFrameTransmitterMock);
    // receive a single frame message
    uint8_t data2[] = {0xab, 0xcd, 0xef, 0x19, 0x28};
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x23, 0x14, sizeof(data2), _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&_transportMessage2),
            Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
    ITransportMessageProcessedListener* notificationListener = 0L;
    EXPECT_CALL(
        _messageProvidingListenerMock, messageReceived(_busId, Ref(_transportMessage2), NotNull()))
        .WillOnce(DoAll(
            SaveArg<2>(&notificationListener),
            Return(ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)));
    // receive the single frame
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x4321, 0x8765),
            DoCanTransportAddressPair(0x23, 0x14)),
        sizeof(data2),
        1U,
        7U,
        data2);
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
    Mock::VerifyAndClearExpectations(&_flowControlFrameTransmitterMock);
    // receive a second single frame message
    uint8_t data3[] = {0xab, 0xef, 0x19, 0x28};
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x22, 0x44, sizeof(data3), _, _))
        .WillOnce(Return(ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE));
    // receive the single frame
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x4343, 0x8787),
            DoCanTransportAddressPair(0x22, 0x44)),
        sizeof(data3),
        1U,
        7U,
        data3);
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
    Mock::VerifyAndClearExpectations(&_flowControlFrameTransmitterMock);
    // Trigger allocation (by processed callback)
    EXPECT_CALL(_messageProvidingListenerMock, releaseTransportMessage(Ref(_transportMessage2)));
    _context.handleExecute();
    notificationListener->transportMessageProcessed(
        _transportMessage2,
        ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);

    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x22, 0x44, sizeof(data3), _, _))
        .WillOnce(Return(ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE));
    _context.execute();

    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);

    // shutdown
    EXPECT_CALL(_messageProvidingListenerMock, releaseTransportMessage(Ref(_transportMessage1)));
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

/**
 * \req: [BSW_DoCAN_187], [BSW_DoCAN_114], [BSW_DoCAN_80]
 */
TEST_F(DoCanReceiverTest, testReceiveSegmentedMessageWithBlockSizeAndShutdown)
{
    ::util::estd::declare::
        block_pool<5U, sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, 7U>)>
            messageReceiverBlockPool;
    DoCanParameters parameters(
        ::estd::function<uint32_t()>::create<DoCanReceiverTest, &DoCanReceiverTest::systemUs>(
            *this),
        100U,
        200U,
        300U,
        400U,
        2U,
        3U,
        0U,
        1U);
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        parameters,
        _loggerComponent);
    cut.init();

    uint8_t const data[] = {
        0xab, 0xcd, 0xef, 0x19, 0x28, 0x37, 0x46, 0x55, 0x64, 0x73, 0x82, 0x91, 0x11, 0x22, 0x33};
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, sizeof(data), _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&_transportMessage1),
            Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
    EXPECT_CALL(
        _flowControlFrameTransmitterMock, sendFlowControl(_, 0x5678, FlowStatus::CTS, 1U, 0U))
        .WillOnce(Return(true));
    // receive the first frame
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x1234, 0x5678),
            DoCanTransportAddressPair(0x14, 0x23)),
        sizeof(data),
        3U,
        7U,
        ::estd::slice<uint8_t const>::from_pointer(data, 6U));
    Mock::VerifyAndClearExpectations(&_flowControlFrameTransmitterMock);
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
    // receive consecutive frame
    EXPECT_CALL(
        _flowControlFrameTransmitterMock, sendFlowControl(_, 0x5678, FlowStatus::CTS, 1U, 0U))
        .WillOnce(Return(true));
    cut.consecutiveDataFrameReceived(
        0x1234, 1U, ::estd::slice<uint8_t const>::from_pointer(data + 6U, 7U));
    Mock::VerifyAndClearExpectations(&_flowControlFrameTransmitterMock);
    EXPECT_CALL(
        _messageProvidingListenerMock, messageReceived(_busId, Ref(_transportMessage1), NotNull()))
        .WillOnce(Return(ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR));
    cut.consecutiveDataFrameReceived(
        0x1234, 2U, ::estd::slice<uint8_t const>::from_pointer(data + 13U, 2U));
    Mock::VerifyAndClearExpectations(&_flowControlFrameTransmitterMock);
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);

    // shutdown
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

TEST_F(DoCanReceiverTest, testReceptionOfSegmentedMessageIsCancelledByNextFirstFrame)
{
    ::util::estd::declare::
        block_pool<5U, sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, 7U>)>
            messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t const data1[] = {
        0xab, 0xcd, 0xef, 0x19, 0x28, 0x37, 0x46, 0x55, 0x64, 0x73, 0x82, 0x91, 0x11, 0x22, 0x33};
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, sizeof(data1), _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&_transportMessage1),
            Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
    EXPECT_CALL(
        _flowControlFrameTransmitterMock, sendFlowControl(_, 0x5678, FlowStatus::CTS, 0U, 0U))
        .WillOnce(Return(true));
    // receive the first frame
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x1234, 0x5678),
            DoCanTransportAddressPair(0x14, 0x23)),
        sizeof(data1),
        3U,
        7U,
        ::estd::slice<uint8_t const>::from_pointer(data1, 6U));
    // receive consecutive frame
    (cut.consecutiveDataFrameReceived(
        0x1234, 1U, ::estd::slice<uint8_t const>::from_pointer(data1 + 6U, 7U)));
    Mock::VerifyAndClearExpectations(&_flowControlFrameTransmitterMock);
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
    // receive single frame
    uint8_t const data2[] = {0x19, 0x28, 0x37, 0x46};
    expectLog(LEVEL_INFO, 0x1234);
    EXPECT_CALL(_messageProvidingListenerMock, releaseTransportMessage(Ref(_transportMessage1)));
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, sizeof(data2), _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&_transportMessage2),
            Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
    EXPECT_CALL(
        _messageProvidingListenerMock, messageReceived(_busId, Ref(_transportMessage2), NotNull()))
        .WillOnce(Return(ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR));
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x1234, 0x5678),
            DoCanTransportAddressPair(0x14, 0x23)),
        sizeof(data2),
        1U,
        7U,
        data2);
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
    EXPECT_EQ(0x14U, _transportMessage2.getSourceId());
    EXPECT_EQ(0x23U, _transportMessage2.getTargetId());
    EXPECT_EQ(
        0,
        ::estd::memory::compare(
            data2,
            ::estd::slice<uint8_t const>::from_pointer(
                _transportMessage2.getPayload(), _transportMessage2.getPayloadLength())));

    // shutdown should be delayed because processing is pending
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

/**
 * \req: [BSW_DoCAN_187], [BSW_DoCAN_74], [BSW_DoCAN_158]
 */
TEST_F(DoCanReceiverTest, testReceiveSegmentedMessageBiggerThan4095MessageAndShutdown)
{
    constexpr uint16_t MESSAGE_SIZE = 4095 * 2 - 1;
    constexpr uint8_t FRAME_SIZE    = 8;
    constexpr uint16_t FRAMES       = (MESSAGE_SIZE + FRAME_SIZE - 1) / FRAME_SIZE;
    ::util::estd::declare::block_pool<
        FRAMES,
        sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, FRAME_SIZE>)>
        messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t data[MESSAGE_SIZE];
    for (size_t i = 0; i < sizeof(data); i++)
    {
        data[i] = i & 0xFF;
    }
    ::transport::BufferedTransportMessage<MESSAGE_SIZE> transportMessage;

    // receive the first frame
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, sizeof(data), _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&transportMessage),
            Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
    EXPECT_CALL(
        _flowControlFrameTransmitterMock, sendFlowControl(_, 0x5678, FlowStatus::CTS, 0U, 0U))
        .WillOnce(Return(true));
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x1234, 0x5678),
            DoCanTransportAddressPair(0x14, 0x23)),
        sizeof(data),
        FRAMES,
        FRAME_SIZE,
        ::estd::slice<uint8_t const>::from_pointer(data, FRAME_SIZE - 1));

    // Read rest
    EXPECT_CALL(
        _messageProvidingListenerMock, messageReceived(_busId, Ref(transportMessage), NotNull()))
        .WillOnce(Return(ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR));
    size_t i = 1;
    for (; i < (FRAMES - 1); i++)
    {
        (cut.consecutiveDataFrameReceived(
            0x1234,
            i & 0xF, // Valid Sequence values
            ::estd::slice<uint8_t const>::from_pointer(data + (FRAME_SIZE * i) - 1, FRAME_SIZE)));
    }
    // Last should be different size
    (cut.consecutiveDataFrameReceived(
        0x1234,
        i & 0xF,
        ::estd::slice<uint8_t const>::from_pointer(
            data + (FRAME_SIZE * i) - 1, (MESSAGE_SIZE % FRAME_SIZE) + 1)));

    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
    EXPECT_EQ(
        0,
        ::estd::memory::compare(
            data,
            ::estd::slice<uint8_t const>::from_pointer(
                transportMessage.getPayload(), transportMessage.getPayloadLength())));
    EXPECT_EQ(0x14U, transportMessage.getSourceId());
    EXPECT_EQ(0x23U, transportMessage.getTargetId());

    // shutdown
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

/**
 * \req: [BSW_DoCAN_187]
 */
TEST_F(DoCanReceiverTest, testReceiveFirstFrameWithAllocationTimeout)
{
    ::util::estd::declare::
        block_pool<5U, sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, 7U>)>
            messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t data[] = {0x10, 0x08, 0xab, 0xcd};
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, sizeof(data), _, _))
        .Times(2)
        .WillRepeatedly(Return(ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE));
    // receive the first frame
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x1234, 0x5678),
            DoCanTransportAddressPair(0x14, 0x23)),
        sizeof(data),
        1U,
        7U,
        data);
    // No timeout
    nowUs = 99999U;
    cut.cyclicTask(nowUs);
    // expect allocation timeout (only 1 retry expected)
    expectLog(LEVEL_WARN, 0x1234);
    nowUs += 1U;
    cut.cyclicTask(nowUs);

    // shutdown should be delayed because processing is pending
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

TEST_F(DoCanReceiverTest, testReceiveInvalidFirstFrame)
{
    ::util::estd::declare::
        block_pool<5U, sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, 7U>)>
            messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t data[] = {0x08, 0x07, 0xab, 0xcd, 0xef, 0x19, 0x28, 0x34, 0x12, 0x34, 0x56, 0x78, 0x98};
    // receive the first frame
    expectLog(LEVEL_ERROR, 0x1234);
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x1234, 0x5678),
            DoCanTransportAddressPair(0x14, 0x23)),
        sizeof(data),
        1U,
        7U,
        data);

    // shutdown
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

TEST_F(DoCanReceiverTest, testReceiveFirstFrameOfUnknownSender)
{
    ::util::estd::declare::
        block_pool<5U, sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, 7U>)>
            messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t data[] = {0x10, 0x08, 0xab, 0xcd};
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, sizeof(data), _, _))
        .WillOnce(Return(ITransportMessageProvider::ErrorCode::TPMSG_INVALID_SRC_ID));
    // receive the first frame
    expectLog(
        LEVEL_WARN, 0x1234, "DoCanReceiver(%s)::allocateTransportMessage(%s): Illegal source id.");
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x1234, 0x5678),
            DoCanTransportAddressPair(0x14, 0x23)),
        sizeof(data),
        1U,
        7U,
        data);

    // shutdown
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

TEST_F(DoCanReceiverTest, testReceiveFirstFrameOfUnknownReceiver)
{
    ::util::estd::declare::
        block_pool<5U, sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, 7U>)>
            messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t data[] = {0x10, 0x08, 0xab, 0xcd};
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, sizeof(data), _, _))
        .WillOnce(Return(ITransportMessageProvider::ErrorCode::TPMSG_INVALID_TGT_ID));
    // receive the first frame
    expectLog(
        LEVEL_WARN, 0x1234, "DoCanReceiver(%s)::allocateTransportMessage(%s): Illegal target id.");
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x1234, 0x5678),
            DoCanTransportAddressPair(0x14, 0x23)),
        sizeof(data),
        1U,
        7U,
        data);

    // shutdown
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

TEST_F(DoCanReceiverTest, testTransportMessageFailReceiveFirstFrame)
{
    ::util::estd::declare::
        block_pool<5U, sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, 7U>)>
            messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t data[] = {0x10, 0x08, 0xab, 0xcd};
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, sizeof(data), _, _))
        .WillOnce(Return(ITransportMessageProvider::ErrorCode::TPMSG_SIZE_TOO_LARGE));
    // receive the first frame
    expectLog(
        LEVEL_WARN,
        0x1234,
        "DoCanReceiver(%s)::allocateTransportMessage(%s): No buffer available (error: 0x%x). "
        "Message discarded.");
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x1234, 0x5678),
            DoCanTransportAddressPair(0x14, 0x23)),
        sizeof(data),
        1U,
        7U,
        data);

    // shutdown
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

TEST_F(DoCanReceiverTest, testReceiveSingleFrameMessagesWithoutAllocationSuccess)
{
    ::util::estd::declare::
        block_pool<3U, sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, 7U>)>
            messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t data[] = {0x02, 0x01, 0x03};
    // receive the single frame
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, sizeof(data), _, _))
        .WillOnce(Return(ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE));
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    DoCanConnection<DataLinkLayer> connection(
        codec,
        DataLinkLayer::AddressPairType(0x1234, 0x5678),
        DoCanTransportAddressPair(0x14, 0x23));
    for (uint8_t idx = 0; idx < 3U; ++idx)
    {
        if (idx != 0U)
        {
            EXPECT_CALL(
                _flowControlFrameTransmitterMock,
                sendFlowControl(_, 0x5678, FlowStatus::WAIT, 0U, 0U))
                .WillOnce(Return(true));
        }
        if (idx > 1U)
        {
            expectLog(LEVEL_INFO, 0x1234);
        }

        cut.firstDataFrameReceived(connection, sizeof(data), 1U + idx, 7U, data);
        Mock::VerifyAndClearExpectations(&_flowControlFrameTransmitterMock);
        Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
        Mock::VerifyAndClearExpectations(&_componentMappingMock);
    }
    expectLog(LEVEL_INFO, 0x1234);
    EXPECT_CALL(
        _flowControlFrameTransmitterMock, sendFlowControl(_, 0x5678, FlowStatus::WAIT, 0U, 0U))
        .WillOnce(Return(true));
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x1234, 0x5678),
            DoCanTransportAddressPair(0x14, 0x23)),
        sizeof(data),
        2U,
        7U,
        data);
    Mock::VerifyAndClearExpectations(&_flowControlFrameTransmitterMock);
    Mock::VerifyAndClearExpectations(&_componentMappingMock);
    // allocate first message and free second one
    nowUs = 99999U;
    cut.cyclicTask(nowUs);
    Sequence seq;
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, sizeof(data), _, _))
        .InSequence(seq)
        .WillOnce(DoAll(
            SetArgReferee<5>(&_transportMessage1),
            Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
    ITransportMessageProcessedListener* notificationListener = 0L;
    EXPECT_CALL(
        _messageProvidingListenerMock, messageReceived(_busId, Ref(_transportMessage1), NotNull()))
        .InSequence(seq)
        .WillOnce(DoAll(
            SaveArg<2>(&notificationListener),
            Return(ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)));
    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, sizeof(data), _, _))
        .InSequence(seq)
        .WillOnce(Return(ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE));
    EXPECT_CALL(
        _flowControlFrameTransmitterMock, sendFlowControl(_, 0x5678, FlowStatus::WAIT, 0U, 0U));
    nowUs += 1U;
    cut.cyclicTask(nowUs);
    Mock::VerifyAndClearExpectations(&_flowControlFrameTransmitterMock);
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
    // free message and continue segmented message reception
    EXPECT_CALL(_messageProvidingListenerMock, releaseTransportMessage(Ref(_transportMessage1)));
    _context.handleExecute();
    notificationListener->transportMessageProcessed(
        _transportMessage1,
        ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);

    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, sizeof(data), _, _))
        .InSequence(seq)
        .WillOnce(DoAll(
            SetArgReferee<5>(&_transportMessage1),
            Return(ITransportMessageProvider::ErrorCode::TPMSG_OK)));
    EXPECT_CALL(
        _flowControlFrameTransmitterMock, sendFlowControl(_, 0x5678, FlowStatus::CTS, 0U, 0U));
    _context.execute();

    Mock::VerifyAndClearExpectations(&_flowControlFrameTransmitterMock);
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);

    // shutdown
    EXPECT_CALL(_messageProvidingListenerMock, releaseTransportMessage(Ref(_transportMessage1)));
    cut.shutdown();
    ASSERT_TRUE(messageReceiverBlockPool.full());
}

TEST_F(DoCanReceiverTest, testIgnoreFunctionalFirstFrames)
{
    ::util::estd::declare::
        block_pool<3U, sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, 7U>)>
            messageReceiverBlockPool;
    DoCanReceiver<DataLinkLayer> cut(
        _busId,
        _context,
        _messageProvidingListenerMock,
        _flowControlFrameTransmitterMock,
        messageReceiverBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t data[] = {0x02, 0x01, 0x03};

    expectLog(LEVEL_WARN, 0x1234);

    // ignore first frame
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x1234, DataLinkLayer::INVALID_ADDRESS),
            DoCanTransportAddressPair(0x14, 0x23)),
        sizeof(data),
        2U,
        7U,
        data);

    cut.cyclicTask(nowUs);
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);

    EXPECT_CALL(
        _messageProvidingListenerMock, getTransportMessage(_busId, 0x14, 0x23, sizeof(data), _, _))
        .WillOnce(Return(ITransportMessageProvider::ErrorCode::TPMSG_OK));

    // accept single frame
    cut.firstDataFrameReceived(
        DoCanConnection<DataLinkLayer>(
            codec,
            DataLinkLayer::AddressPairType(0x1234, 0x5678),
            DoCanTransportAddressPair(0x14, 0x23)),
        sizeof(data),
        1U,
        7U,
        data);

    cut.cyclicTask(nowUs);
    Mock::VerifyAndClearExpectations(&_messageProvidingListenerMock);
}

void DoCanReceiverTest::expectLog(Level const level)
{
    EXPECT_CALL(_componentMappingMock, isEnabled(_loggerComponent, level)).WillOnce(Return(false));
}

void DoCanReceiverTest::expectLog(Level const level, uint32_t address)
{
    expectLog(level);
    EXPECT_CALL(_addressConverterMock, formatDataLinkAddress(address, _)).WillOnce(Return("abc"));
}

void DoCanReceiverTest::expectLog(Level const level, uint32_t address, char const* logMessage)
{
    EXPECT_CALL(_componentMappingMock, isEnabled(_loggerComponent, level)).WillOnce(Return(true));
    EXPECT_CALL(_componentMappingMock, getLevelInfo(level)).Times(1);
    EXPECT_CALL(_componentMappingMock, getComponentInfo(_)).Times(1);
    EXPECT_CALL(_loggerOutputMock, logOutput(_, _, logMessage, _)).Times(1);
    EXPECT_CALL(_addressConverterMock, formatDataLinkAddress(address, _)).WillOnce(Return("abc"));
}
} // anonymous namespace
