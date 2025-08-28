// Copyright 2024 Accenture.

#include "docan/transmitter/DoCanTransmitter.h"

#include "docan/addressing/DoCanAddressConverterMock.h"
#include "docan/common/DoCanConstants.h"
#include "docan/common/DoCanJobHandle.h"
#include "docan/datalink/DoCanDataFrameTransmitterMock.h"
#include "docan/datalink/DoCanDataLinkAddressPair.h"
#include "docan/datalink/DoCanDefaultFrameSizeMapper.h"
#include "docan/datalink/DoCanFrameCodec.h"
#include "docan/datalink/DoCanFrameCodecConfigPresets.h"
#include "docan/transmitter/DoCanTickGeneratorMock.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <bsp/timer/SystemTimerMock.h>
#include <etl/delegate.h>
#include <etl/generic_pool.h>
#include <etl/memory.h>
#include <etl/span.h>
#include <transport/BufferedTransportMessage.h>
#include <transport/TransportMessageProcessedListenerMock.h>
#include <util/logger/ComponentMappingMock.h>
#include <util/logger/LoggerOutputMock.h>

#include <gtest/esr_extensions.h>

namespace
{
using namespace docan;
using namespace ::util::logger;
using namespace testing;
using namespace transport;

struct DataLinkLayer
{
    static uint32_t const INVALID_ADDRESS = 0XFFFFFFFFU;

    using AddressType     = uint32_t;
    using AddressPairType = DoCanDataLinkAddressPair<uint32_t>;
    using MessageSizeType = uint16_t;
    using FrameIndexType  = uint16_t;
    using FrameSizeType   = uint8_t;
    using JobHandleType   = DoCanJobHandle<uint16_t, uint16_t>;
};

struct SmallFrameIndexDataLinkLayer
{
    static uint32_t const INVALID_ADDRESS = 0XFFFFFFFFU;

    using AddressType     = uint32_t;
    using AddressPairType = DoCanDataLinkAddressPair<uint32_t>;
    using MessageSizeType = uint16_t;
    using FrameIndexType  = uint8_t; // uint8 instead of uint16
    using FrameSizeType   = uint8_t;
    using JobHandleType   = DoCanJobHandle<uint16_t, uint16_t>;
};

using JobHandle = DataLinkLayer::JobHandleType;
using CodecType = DoCanFrameCodec<DataLinkLayer>;

struct DoCanTransmitterTest : ::testing::Test
{
    static uint16_t const waitAllocateTimeout    = 100U;
    static uint16_t const waitRxTimeout          = 200U;
    static uint16_t const waitTxCallbackTimeout  = 300U;
    static uint16_t const waitFlowControlTimeout = 400U;
    static uint8_t const maxAllocateRetryCount   = 2U;
    static uint8_t const maxFlowControlWaitCount = 3U;
    static uint16_t const minSeparationTime      = 0U;
    static uint8_t const maxBlockSize            = 0U;

    DoCanTransmitterTest()
    : _parameters(
        ::etl::delegate<uint32_t()>::create<DoCanTransmitterTest, &DoCanTransmitterTest::systemUs>(
            *this),
        waitAllocateTimeout,
        waitRxTimeout,
        waitTxCallbackTimeout,
        waitFlowControlTimeout,
        maxAllocateRetryCount,
        maxFlowControlWaitCount,
        minSeparationTime,
        maxBlockSize)
    , nowUs(0)
    , _loggerComponent(8)
    , _codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, _mapper)
    , _context(1)
    {
        EXPECT_CALL(_systemTimer, getSystemTimeNs()).WillRepeatedly(testing::Return(1000000UL));
        Logger::init(_componentMappingMock, _loggerOutputMock);
    }

    ~DoCanTransmitterTest() override { Logger::shutdown(); }

    template<size_t N>
    void initMessage(
        TransportMessage& message,
        DoCanTransportAddressPair const tPair,
        DataLinkLayer::AddressPairType const aPair,
        uint8_t (&buffer)[N])
    {
        Mock::VerifyAndClearExpectations(&_addressConverterMock);
        message.init(buffer, N);
        message.setPayloadLength(N);
        message.increaseValidBytes(N);
        message.setSourceId(tPair.getSourceId());
        message.setTargetId(tPair.getTargetId());
        EXPECT_CALL(_addressConverterMock, getTransmissionParameters(tPair, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(aPair), Return(&_codec)));
    }

    void expectLog(Level level);
    void expectLog(Level level, uint32_t address);
    void expectLog(Level level, char const* logMessage);

    uint32_t systemUs() { return nowUs; }

    using DataFrameTransmitterType = IDoCanDataFrameTransmitter<DataLinkLayer>;

    StrictMock<ComponentMappingMock> _componentMappingMock;
    StrictMock<LoggerOutputMock> _loggerOutputMock;
    StrictMock<DoCanDataFrameTransmitterMock<DataLinkLayer>> _dataFrameTransmitterMock;
    StrictMock<DoCanTickGeneratorMock> _tickGeneratorMock;
    StrictMock<TransportMessageProcessedListenerMock> _processedListenerMock;
    StrictMock<DoCanAddressConverterMock<DataLinkLayer>> _addressConverterMock;
    DoCanParameters _parameters;
    ::testing::SystemTimerMock _systemTimer;
    uint8_t _busId = 0xFF;
    uint32_t nowUs;
    uint8_t _loggerComponent;
    CodecType _codec;
    DoCanDefaultFrameSizeMapper<uint8_t> const _mapper;
    ::async::AsyncMock asyncMock;
    ::async::TestContext _context;
};

using ItemT = ::docan::DoCanMessageTransmitter<DataLinkLayer>;

/**
 * \req: [BSW_DoCAN_186], [BSW_DoCAN_72]
 */
TEST_F(DoCanTransmitterTest, testTransmitSingleFrameMessageAndShutdown)
{
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 5U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t data[] = {0xab, 0xcd, 0xef, 0x19, 0x28};
    TransportMessage message;
    auto const addrPair      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message, transportPair, addrPair, data);
    // send the message
    _context.handleExecute();
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message, &_processedListenerMock));
    // expect first send when looping
    JobHandle jobHandle(0xff, 0x99);
    IDoCanDataFrameTransmitterCallback<DataLinkLayer>* callback = nullptr;
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _, _, _, addrPair.getTransmissionAddress(), 0U, 1U, 0U, ElementsAreArray(data)))
        .WillOnce(DoAll(
            WithArg<1>(SaveRef<0>(&callback)),
            SaveArg<2>(&jobHandle),
            Return(SendResult::QUEUED_FULL)));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // frames sent
    callback->dataFramesSent(jobHandle, 1U, sizeof(data));
    // expect result from context
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));
    _context.execute();

    ASSERT_TRUE(messageTransmitterBlockPool.empty());
    cut.shutdown();
}

TEST_F(DoCanTransmitterTest, testTransmitMultipleSingleFrameMessageExpectDifferentJobHandles)
{
    constexpr size_t NUMBER_OF_SLOTS = 3;
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), NUMBER_OF_SLOTS> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();
    constexpr size_t NUMBER_OF_MESSAGES = NUMBER_OF_SLOTS * 3;
    JobHandle jobHandles[NUMBER_OF_MESSAGES];
    for (size_t i = 0; i < NUMBER_OF_MESSAGES; i++)
    {
        uint8_t data[] = {0xab, 0xcd, 0xef, 0x19, 0x28};
        TransportMessage message;
        auto const addrPair      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
        auto const transportPair = DoCanTransportAddressPair(0x45, 0x54);
        initMessage(message, transportPair, addrPair, data);
        // send the message
        _context.handleExecute();
        ASSERT_EQ(
            ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
            cut.send(message, &_processedListenerMock));
        // expect first send on async execution
        IDoCanDataFrameTransmitterCallback<DataLinkLayer>* callback = nullptr;
        EXPECT_CALL(
            _dataFrameTransmitterMock,
            startSendDataFrames(
                _, _, _, addrPair.getTransmissionAddress(), 0U, 1U, 0U, ElementsAreArray(data)))
            .WillOnce(DoAll(
                WithArg<1>(SaveRef<0>(&callback)),
                SaveArg<2>(&jobHandles[i]),
                Return(SendResult::QUEUED_FULL)));
        _context.execute();
        for (size_t j = 0; j < i; j++)
        {
            ASSERT_NE(jobHandles[j], jobHandles[i]);
        }

        Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
        // frames sent
        callback->dataFramesSent(jobHandles[i], 1U, sizeof(data));
        // expect result from context
        EXPECT_CALL(
            _processedListenerMock,
            transportMessageProcessed(
                Ref(message),
                ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));
        _context.execute();

        ASSERT_TRUE(messageTransmitterBlockPool.empty());
    }
    cut.shutdown();
}

TEST_F(DoCanTransmitterTest, testTransmitSingleFrameMessageWithoutNotificationListenerAndShutdown)
{
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 5U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t data[] = {0xab, 0xcd, 0xef, 0x19, 0x28};
    TransportMessage message;
    auto const addrPair      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message, transportPair, addrPair, data);
    _context.handleExecute();
    // send the message
    ASSERT_EQ(::transport::AbstractTransportLayer::ErrorCode::TP_OK, cut.send(message, nullptr));
    // expect first send when looping
    JobHandle jobHandle(0xff, 0x99);
    IDoCanDataFrameTransmitterCallback<DataLinkLayer>* callback = nullptr;
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _, _, _, addrPair.getTransmissionAddress(), 0U, 1U, 0U, ElementsAreArray(data)))
        .WillOnce(DoAll(
            WithArg<1>(SaveRef<0>(&callback)),
            SaveArg<2>(&jobHandle),
            Return(SendResult::QUEUED_FULL)));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // frames sent
    callback->dataFramesSent(jobHandle, 1U, sizeof(data));
    // expect no result from context
    _context.execute();
    ASSERT_TRUE(messageTransmitterBlockPool.empty());
    cut.shutdown();
}

/**
 * \req: [BSW_DoCAN_186], [BSW_DoCAN_70]
 */
TEST_F(DoCanTransmitterTest, testTransmitSegmentedMessageAndShutdown)
{
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 5U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);

    cut.init();
    uint8_t data[] = {
        0xab, 0xcd, 0xef, 0x19, 0x28, 0x98, 0xa1, 0x45, 0x11, 0x22, 0x33, 0x44, 0x55, 0x67, 0x9e};
    TransportMessage message;
    auto const addrPair      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message, transportPair, addrPair, data);
    // send the segmented message
    _context.handleExecute();
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message, &_processedListenerMock));
    // expect first send when looping
    JobHandle jobHandle(0x1f, 0x99);
    IDoCanDataFrameTransmitterCallback<DataLinkLayer>* callback = nullptr;
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _, _, _, addrPair.getTransmissionAddress(), 0U, 1U, 7U, ElementsAreArray(data)))
        .WillOnce(DoAll(
            WithArg<1>(SaveRef<0>(&callback)),
            SaveArg<2>(&jobHandle),
            Return(SendResult::QUEUED_FULL)));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // frames sent
    callback->dataFramesSent(jobHandle, 1U, 6U);
    // Expect immediate send after flow control
    ::etl::span<uint8_t const> span = ::etl::span<uint8_t const>(data).subspan(6U);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle,
            addrPair.getTransmissionAddress(),
            1U,
            3U,
            7U,
            ElementsAreArray(span.data(), span.size())))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    cut.flowControlFrameReceived(addrPair.getReceptionAddress(), FlowStatus::CTS, 0U, 0U);
    // Expect immediate send after frames sent
    ::etl::span<uint8_t const> span2 = ::etl::span<uint8_t const>(data).subspan(13U);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle,
            addrPair.getTransmissionAddress(),
            2U,
            3U,
            7U,
            ElementsAreArray(span2.data(), span2.size())))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    callback->dataFramesSent(jobHandle, 1U, 7U);
    // No direct response after last frame sent
    callback->dataFramesSent(jobHandle, 1U, 2U);
    // expect result from context
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));
    _context.execute();

    ASSERT_TRUE(messageTransmitterBlockPool.empty());
    cut.shutdown();
}

TEST_F(DoCanTransmitterTest, testTransmitSegmentedMessageWithFlowControlOverflowAndShutdown)
{
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 5U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);

    cut.init();
    uint8_t data[] = {
        0xab, 0xcd, 0xef, 0x19, 0x28, 0x98, 0xa1, 0x45, 0x11, 0x22, 0x33, 0x44, 0x55, 0x67, 0x9e};
    TransportMessage message;
    auto const addrPair      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message, transportPair, addrPair, data);
    _context.handleExecute();
    // send the segmented message
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message, &_processedListenerMock));
    // expect first send when looping
    JobHandle jobHandle(0x1f, 0x99);
    IDoCanDataFrameTransmitterCallback<DataLinkLayer>* callback = nullptr;
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _, _, _, addrPair.getTransmissionAddress(), 0U, 1U, 7U, ElementsAreArray(data)))
        .WillOnce(DoAll(
            WithArg<1>(SaveRef<0>(&callback)),
            SaveArg<2>(&jobHandle),
            Return(SendResult::QUEUED_FULL)));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // frames sent
    callback->dataFramesSent(jobHandle, 1U, 6U);
    expectLog(LEVEL_WARN, "DoCanTransmitter(%s)::%s(0x%x -> 0x%x): Flow control overflow received");
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR_OVERFLOW));
    cut.flowControlFrameReceived(addrPair.getReceptionAddress(), FlowStatus::OVFLW, 0U, 0U);
    Mock::VerifyAndClearExpectations(&_processedListenerMock);

    ASSERT_TRUE(messageTransmitterBlockPool.empty());
    cut.shutdown();
}

TEST_F(DoCanTransmitterTest, testTransmitSegmentedMessageWithInvalidFlowControlAndShutdown)
{
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 5U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);

    cut.init();
    uint8_t data[] = {
        0xab, 0xcd, 0xef, 0x19, 0x28, 0x98, 0xa1, 0x45, 0x11, 0x22, 0x33, 0x44, 0x55, 0x67, 0x9e};
    TransportMessage message;
    auto const addrPair      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message, transportPair, addrPair, data);
    _context.handleExecute();
    // send the segmented message
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message, &_processedListenerMock));
    // expect first send when looping
    JobHandle jobHandle(0x1f, 0x99);
    IDoCanDataFrameTransmitterCallback<DataLinkLayer>* callback = nullptr;
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _, _, _, addrPair.getTransmissionAddress(), 0U, 1U, 7U, ElementsAreArray(data)))
        .WillOnce(DoAll(
            WithArg<1>(SaveRef<0>(&callback)),
            SaveArg<2>(&jobHandle),
            Return(SendResult::QUEUED_FULL)));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // frames sent
    callback->dataFramesSent(jobHandle, 1U, 6U);
    expectLog(LEVEL_WARN, "DoCanTransmitter(%s)::%s(0x%x -> 0x%x): Invalid flow control received");
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR_ABORT));
    cut.flowControlFrameReceived(
        addrPair.getReceptionAddress(), static_cast<FlowStatus>(4U), 0U, 0U);
    Mock::VerifyAndClearExpectations(&_processedListenerMock);

    ASSERT_TRUE(messageTransmitterBlockPool.empty());
    cut.shutdown();
}

TEST_F(DoCanTransmitterTest, testTransmitSegmentedMessageWithFlowControlWaitOverflowAndShutdown)
{
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 5U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);

    cut.init();
    uint8_t data[] = {
        0xab, 0xcd, 0xef, 0x19, 0x28, 0x98, 0xa1, 0x45, 0x11, 0x22, 0x33, 0x44, 0x55, 0x67, 0x9e};
    TransportMessage message;
    auto const addrPair      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message, transportPair, addrPair, data);
    _context.handleExecute();
    // send the segmented message
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message, &_processedListenerMock));
    // expect first send when looping
    JobHandle jobHandle(0x1f, 0x99);
    IDoCanDataFrameTransmitterCallback<DataLinkLayer>* callback = nullptr;
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _, _, _, addrPair.getTransmissionAddress(), 0U, 1U, 7U, ElementsAreArray(data)))
        .WillOnce(DoAll(
            WithArg<1>(SaveRef<0>(&callback)),
            SaveArg<2>(&jobHandle),
            Return(SendResult::QUEUED_FULL)));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // frames sent
    callback->dataFramesSent(jobHandle, 1U, 6U);
    // Expect nothing maxFlowControlWaitCount times.
    for (size_t i = 0; i < maxFlowControlWaitCount; i++)
    {
        cut.flowControlFrameReceived(addrPair.getReceptionAddress(), FlowStatus::WAIT, 0U, 0U);
    }
    expectLog(
        LEVEL_WARN, "DoCanTransmitter(%s)::%s(0x%x -> 0x%x): Flow control wait count exceeded");
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR_GENERAL));
    cut.flowControlFrameReceived(addrPair.getReceptionAddress(), FlowStatus::WAIT, 0U, 0U);
    Mock::VerifyAndClearExpectations(&_processedListenerMock);

    ASSERT_TRUE(messageTransmitterBlockPool.empty());
    cut.shutdown();
}

/**
 * \req: [BSW_DoCAN_186]
 */
TEST_F(DoCanTransmitterTest, testTransmitSegmentedMessageWithSeparationTimeAndShutdown)
{
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 5U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t data[] = {
        0xab, 0xcd, 0xef, 0x19, 0x28, 0x98, 0xa1, 0x45, 0x11, 0x22, 0x33, 0x44, 0x55, 0x67, 0x9e};
    TransportMessage message;
    auto const addrPair      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message, transportPair, addrPair, data);
    // send the segmented message
    _context.handleExecute();
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message, &_processedListenerMock));
    // expect first send when looping
    IDoCanDataFrameTransmitterCallback<DataLinkLayer>* callback = nullptr;
    JobHandle jobHandle(0x2f, 0x99);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _, _, _, addrPair.getTransmissionAddress(), 0U, 1U, 7U, ElementsAreArray(data)))
        .WillOnce(DoAll(
            WithArg<1>(SaveRef<0>(&callback)),
            SaveArg<2>(&jobHandle),
            Return(SendResult::QUEUED_FULL)));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // frames sent
    callback->dataFramesSent(jobHandle, 1U, 9U);
    ::etl::span<uint8_t const> span = ::etl::span<uint8_t const>(data).subspan(9U);
    // Expect immediate send after flow control
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle,
            addrPair.getTransmissionAddress(),
            1U,
            2U,
            7U,
            ElementsAreArray(span.data(), span.size())))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    cut.flowControlFrameReceived(addrPair.getReceptionAddress(), FlowStatus::CTS, 0U, 2U);
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // Expect no immediate send after frames sent
    EXPECT_CALL(_tickGeneratorMock, tickNeeded());
    callback->dataFramesSent(jobHandle, 1U, 3U);
    cut.cyclicTask(nowUs);
    cut.cyclicTask(nowUs);
    span = ::etl::span<uint8_t const>(data).subspan(12U);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle,
            addrPair.getTransmissionAddress(),
            2U,
            3U,
            7U,
            ElementsAreArray(span.data(), span.size())))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    nowUs = 2000;
    cut.cyclicTask(nowUs);
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    callback->dataFramesSent(jobHandle, 1U, 3U);
    // expect result from context
    callback->dataFramesSent(jobHandle, 1U, 3U);
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));
    _context.execute();

    ASSERT_TRUE(messageTransmitterBlockPool.empty());
    cut.shutdown();
}

TEST_F(DoCanTransmitterTest, sequentialFramesSentCorrectlyWithNowUsWraparound)
{
    nowUs = std::numeric_limits<uint32_t>::max() - 999;
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 5U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t data[] = {
        0xab, 0xcd, 0xef, 0x19, 0x28, 0x98, 0xa1, 0x45, 0x11, 0x22, 0x33, 0x44, 0x55, 0x67, 0x9e};
    TransportMessage message;
    auto const addrPair      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message, transportPair, addrPair, data);
    // send the segmented message
    _context.handleExecute();
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message, &_processedListenerMock));
    // expect first send when looping
    IDoCanDataFrameTransmitterCallback<DataLinkLayer>* callback = nullptr;
    JobHandle jobHandle(0x2f, 0x99);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(_, _, _, 0x5678, 0U, 1U, 7U, ElementsAreArray(data)))
        .WillOnce(DoAll(
            WithArg<1>(SaveRef<0>(&callback)),
            SaveArg<2>(&jobHandle),
            Return(SendResult::QUEUED_FULL)));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // frames sent
    callback->dataFramesSent(jobHandle, 1U, 9U);
    ::etl::span<uint8_t const> span = ::etl::span<uint8_t const>(data).subspan(9U);
    // Expect immediate send after flow control
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle,
            0x5678,
            1U,
            2U,
            7U,
            ElementsAreArray(span.data(), span.size())))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    // Set an STmin of 2ms
    cut.flowControlFrameReceived(0x1234U, FlowStatus::CTS, 0U, 2U);
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // Expect no immediate send after frames sent
    EXPECT_CALL(_tickGeneratorMock, tickNeeded());
    callback->dataFramesSent(jobHandle, 1U, 3U);
    cut.cyclicTask(nowUs);
    // Ensure we don't send our first sequential frame until at least 2ms has passed, even with
    // timer wraparound
    nowUs = 999;
    cut.cyclicTask(nowUs);
    span = ::etl::span<uint8_t const>(data).subspan(12U);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle,
            0x5678,
            2U,
            3U,
            7U,
            ElementsAreArray(span.data(), span.size())))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    nowUs = 1000;
    cut.cyclicTask(nowUs);
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    callback->dataFramesSent(jobHandle, 1U, 3U);
    // expect result from context
    callback->dataFramesSent(jobHandle, 1U, 3U);
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));
    _context.execute();

    ASSERT_TRUE(messageTransmitterBlockPool.empty());
    cut.shutdown();
}

/**
 * \req: [BSW_DoCAN_186]
 */
TEST_F(
    DoCanTransmitterTest,
    testTransmitSegmentedMessageWithMultipleConsecutiveFramesAtOnceAndShutdown)
{
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 5U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t data[] = {
        0xab, 0xcd, 0xef, 0x19, 0x28, 0x98, 0xa1, 0x45, 0x11, 0x22, 0x33, 0x44, 0x55, 0x67, 0x9e};
    TransportMessage message;
    auto const addrPair      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message, transportPair, addrPair, data);
    // send the segmented message
    _context.handleExecute();
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message, &_processedListenerMock));
    // expect first send when looping
    IDoCanDataFrameTransmitterCallback<DataLinkLayer>* callback = nullptr;
    JobHandle jobHandle(0x3f, 0x99);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _, _, _, addrPair.getTransmissionAddress(), 0U, 1U, 7U, ElementsAreArray(data)))
        .WillOnce(DoAll(
            WithArg<1>(SaveRef<0>(&callback)),
            SaveArg<2>(&jobHandle),
            Return(SendResult::QUEUED_FULL)));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // frames sent
    callback->dataFramesSent(jobHandle, 1U, 6U);
    // Expect immediate send after flow control
    ::etl::span<uint8_t const> span = ::etl::span<uint8_t const>(data).subspan(6U);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle,
            addrPair.getTransmissionAddress(),
            1U,
            3U,
            7U,
            ElementsAreArray(span.data(), span.size())))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    cut.flowControlFrameReceived(addrPair.getReceptionAddress(), FlowStatus::CTS, 0U, 0U);
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    callback->dataFramesSent(jobHandle, 2U, 6U);
    // expect result from context
    callback->dataFramesSent(jobHandle, 1U, 3U);
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));
    _context.execute();

    ASSERT_TRUE(messageTransmitterBlockPool.empty());
    cut.shutdown();
}

/**
 * \req: [BSW_DoCAN_186]
 */
TEST_F(
    DoCanTransmitterTest,
    testTransmitSegmentedMessageWithMultipleConsecutiveFramesAtOnceAndEscapeSequenceAndShutdown)
{
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 105U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    constexpr auto MESSAGE_SIZE = 0x1A2BU;
    constexpr auto FRAME_SIZE   = 7U;
    constexpr auto FRAMES       = (MESSAGE_SIZE / FRAME_SIZE) + 1;
    uint8_t data[MESSAGE_SIZE];
    for (size_t i = 0; i < MESSAGE_SIZE; i++)
    {
        data[i] = i & 0xFF;
    }
    ::etl::span<uint8_t const> span(data);
    TransportMessage message;
    auto const addrPair      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message, transportPair, addrPair, data);
    // send the segmented message
    _context.handleExecute();
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message, &_processedListenerMock));
    // expect first send when looping
    IDoCanDataFrameTransmitterCallback<DataLinkLayer>* callback = nullptr;
    JobHandle jobHandle(0x3f, 0x99);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _, _, _, addrPair.getTransmissionAddress(), 0U, 1U, 7U, ElementsAreArray(data)))
        .WillOnce(DoAll(
            WithArg<1>(SaveRef<0>(&callback)),
            SaveArg<2>(&jobHandle),
            Return(SendResult::QUEUED_FULL)));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // frames sent
    callback->dataFramesSent(jobHandle, 1U, 2U);
    span.advance(2U);

    // Expect immediate send after flow control
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle,
            addrPair.getTransmissionAddress(),
            1U,
            FRAMES,
            7U,
            ElementsAreArray(span.data(), span.size())))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    cut.flowControlFrameReceived(addrPair.getReceptionAddress(), FlowStatus::CTS, 0U, 0U);
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);

    callback->dataFramesSent(jobHandle, FRAMES, span.size());
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));
    _context.execute();

    ASSERT_TRUE(messageTransmitterBlockPool.empty());
    cut.shutdown();
}

/**
 * \req: [BSW_DoCAN_186]
 */
TEST_F(DoCanTransmitterTest, testSendTwoSegmentedMessagesRoundRobin)
{
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 5U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    // 3 segment message
    uint8_t data1[] = {
        0xab, 0xcd, 0xef, 0x19, 0x28, 0x98, 0xa1, 0x45, 0x11, 0x22, 0x33, 0x44, 0x55, 0x67, 0x9e};
    TransportMessage message1;
    auto const addrPair1      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair1 = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message1, transportPair1, addrPair1, data1);
    // 4 segment message
    // send the segmented message
    _context.handleExecute();
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message1, &_processedListenerMock));

    uint8_t data2[] = {0xa1, 0x45, 0x11, 0x22, 0x33, 0x44, 0x55, 0x67, 0x9e, 0xab, 0xcd,
                       0xef, 0x19, 0x28, 0x98, 0x91, 0x82, 0x73, 0x64, 0x55, 0x31};
    TransportMessage message2;
    auto const addrPair2      = DataLinkLayer::AddressPairType(0x666, 0x999);
    auto const transportPair2 = DoCanTransportAddressPair(0x43, 0x34);
    initMessage(message2, transportPair2, addrPair2, data2);
    _context.handleExecute();
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message2, &_processedListenerMock));
    // expect first frame of message 1 from context
    IDoCanDataFrameTransmitterCallback<DataLinkLayer>* callback = nullptr;
    JobHandle jobHandle1(0x25, 0x19);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _, _, _, addrPair1.getTransmissionAddress(), 0U, 1U, 7U, ElementsAreArray(data1)))
        .WillOnce(DoAll(
            WithArg<1>(SaveRef<0>(&callback)),
            SaveArg<2>(&jobHandle1),
            Return(SendResult::QUEUED_FULL)));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // expect first frame of message 2 when frames are sent
    JobHandle jobHandle2(0x26, 0x19);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            _,
            addrPair2.getTransmissionAddress(),
            0U,
            1U,
            7U,
            ElementsAreArray(data2)))
        .WillOnce(DoAll(SaveArg<2>(&jobHandle2), Return(SendResult::QUEUED_FULL)));
    callback->dataFramesSent(jobHandle1, 1U, 6U);
    EXPECT_NE(jobHandle1, jobHandle2);
    // expect next frames of message 2 when flow control and no more sent
    cut.flowControlFrameReceived(addrPair2.getReceptionAddress(), FlowStatus::CTS, 0U, 0U);
    ::etl::span<uint8_t const> span2 = ::etl::span<uint8_t const>(data2).subspan(6U);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle2,
            addrPair2.getTransmissionAddress(),
            1U,
            4U,
            7U,
            ElementsAreArray(span2.data(), span2.size())))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    callback->dataFramesSent(jobHandle2, 1U, 6U);
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // expect next frames of message 1 (round robin)
    cut.flowControlFrameReceived(addrPair1.getReceptionAddress(), FlowStatus::CTS, 0U, 0U);
    ::etl::span<uint8_t const> span1 = ::etl::span<uint8_t const>(data1).subspan(6U);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle1,
            addrPair1.getTransmissionAddress(),
            1U,
            3U,
            7U,
            ElementsAreArray(span1.data(), span1.size())))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    callback->dataFramesSent(jobHandle2, 1U, 7U);
    // expect next frames of message 2 (round robin)
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle2,
            addrPair2.getTransmissionAddress(),
            2U,
            4U,
            7U,
            ElementsAreArray(::etl::span<uint8_t const>(data2).subspan(13U))))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    callback->dataFramesSent(jobHandle1, 1U, 7U);
    // expect next frames of message 1 (round robin)
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle1,
            addrPair1.getTransmissionAddress(),
            2U,
            3U,
            7U,
            ElementsAreArray(::etl::span<uint8_t const>(data1).subspan(13U))))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    callback->dataFramesSent(jobHandle2, 1U, 7U);
    // expect next frames of message 2 (round robin)
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle2,
            addrPair2.getTransmissionAddress(),
            3U,
            4U,
            7U,
            ElementsAreArray(::etl::span<uint8_t const>(data2).subspan(20U))))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    callback->dataFramesSent(jobHandle1, 1U, 2U);
    // expect message 1 to be released (from context)
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message1),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));
    _context.execute();
    // expect message 2 to be released...
    callback->dataFramesSent(jobHandle2, 1U, 1U);
    // ... from context
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message2),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));
    _context.execute();

    // shutdown
    ASSERT_TRUE(messageTransmitterBlockPool.empty());
    cut.shutdown();
}

/**
 * \req: [BSW_DoCAN_186]
 */
TEST_F(DoCanTransmitterTest, testSendThreeSegmentedMessagesRoundRobinWithTwoSendSlots)
{
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 5U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    // 3 segment message
    uint8_t data1[] = {
        0xab, 0xcd, 0xef, 0x19, 0x28, 0x98, 0xa1, 0x45, 0x11, 0x22, 0x33, 0x44, 0x55, 0x67, 0x9e};
    TransportMessage message1;
    auto const addrPair1      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair1 = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message1, transportPair1, addrPair1, data1);
    _context.handleExecute();
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message1, &_processedListenerMock));
    // 4 segment message
    uint8_t data2[] = {0xa1, 0x45, 0x11, 0x22, 0x33, 0x44, 0x55, 0x67, 0x9e, 0xab, 0xcd,
                       0xef, 0x19, 0x28, 0x98, 0x91, 0x82, 0x73, 0x64, 0x55, 0x31};
    TransportMessage message2;
    auto const addrPair2      = DataLinkLayer::AddressPairType(0x1235, 0x6678);
    auto const transportPair2 = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message2, transportPair2, addrPair2, data2);

    _context.handleExecute();
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message2, &_processedListenerMock));
    // 3 segment message
    uint8_t data3[]
        = {0x33,
           0x44,
           0x55,
           0x67,
           0x9e,
           0xab,
           0xcd,
           0xef,
           0x19,
           0x28,
           0x98,
           0x91,
           0x82,
           0x73,
           0x64,
           0x55,
           0x31};
    TransportMessage message3;
    auto const addrPair3      = DataLinkLayer::AddressPairType(0x1236, 0x7678);
    auto const transportPair3 = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message3, transportPair3, addrPair3, data3);

    // send the segmented message

    _context.handleExecute();
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message3, &_processedListenerMock));
    // expect first frame of message 1 and first frame of message 2 from context
    IDoCanDataFrameTransmitterCallback<DataLinkLayer>* callback = nullptr;
    JobHandle jobHandle1(0x36, 0x19);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _, _, _, addrPair1.getTransmissionAddress(), 0U, 1U, 7U, ElementsAreArray(data1)))
        .WillOnce(DoAll(
            WithArg<1>(SaveRef<0>(&callback)),
            SaveArg<2>(&jobHandle1),
            Return(SendResult::QUEUED)));
    JobHandle jobHandle2(0x16, 0x19);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _, _, _, addrPair2.getTransmissionAddress(), 0U, 1U, 7U, ElementsAreArray(data2)))
        .WillOnce(DoAll(SaveArg<2>(&jobHandle2), Return(SendResult::QUEUED_FULL)));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    cut.flowControlFrameReceived(addrPair2.getReceptionAddress(), FlowStatus::CTS, 0U, 0U);
    // expect first frame of message 3 when second send job has been done
    JobHandle jobHandle3(0x36, 0x19);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            _,
            addrPair3.getTransmissionAddress(),
            0U,
            1U,
            7U,
            ElementsAreArray(data3)))
        .WillOnce(DoAll(SaveArg<2>(&jobHandle3), Return(SendResult::QUEUED_FULL)));
    callback->dataFramesSent(jobHandle2, 1U, 6U);
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // expect second frame of message 2 when frames have been sent
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle2,
            addrPair2.getTransmissionAddress(),
            1U,
            4U,
            7U,
            ElementsAreArray(::etl::span<uint8_t>(data2).subspan(6U))))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    callback->dataFramesSent(jobHandle3, 1U, 6U);
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // expect third frame of message 2 when frames have been sent
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle2,
            addrPair2.getTransmissionAddress(),
            2U,
            4U,
            7U,
            ElementsAreArray(::etl::span<uint8_t const>(data2).subspan(13U))))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    callback->dataFramesSent(jobHandle2, 1U, 7U);
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // expect second frame of message 1 when flow control is received and data frames have been sent
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle1,
            addrPair1.getTransmissionAddress(),
            1U,
            3U,
            7U,
            ElementsAreArray(::etl::span<uint8_t const>(data1).subspan(6U))))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    cut.flowControlFrameReceived(addrPair1.getReceptionAddress(), FlowStatus::CTS, 0U, 0U);
    callback->dataFramesSent(jobHandle1, 1U, 6U);
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // expect last frame of message 2 when flow control is received and data frames have been sent
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle2,
            addrPair2.getTransmissionAddress(),
            3U,
            4U,
            7U,
            ElementsAreArray(::etl::span<uint8_t const>(data2).subspan(20U))))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    cut.flowControlFrameReceived(addrPair3.getReceptionAddress(), FlowStatus::CTS, 0U, 0U);
    callback->dataFramesSent(jobHandle2, 1U, 7U);
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // expect second frame of message 3 when job 2 has been done
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle3,
            addrPair3.getTransmissionAddress(),
            1U,
            3U,
            7U,
            ElementsAreArray(::etl::span<uint8_t const>(data3).subspan(6U))))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    callback->dataFramesSent(jobHandle2, 1U, 1U);
    // message 2 should be processed now
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message2),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // expect last frame of message 1 when job 1 has been done
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle1,
            addrPair1.getTransmissionAddress(),
            2U,
            3U,
            7U,
            ElementsAreArray(::etl::span<uint8_t>(data1).subspan(13U))))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    callback->dataFramesSent(jobHandle1, 1U, 7U);
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // expect last frame of message 3 when job 3 has been done
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle3,
            addrPair3.getTransmissionAddress(),
            2U,
            3U,
            7U,
            ElementsAreArray(::etl::span<uint8_t const>(data3).subspan(13U))))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    callback->dataFramesSent(jobHandle3, 1U, 7U);
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // all frames sent
    callback->dataFramesSent(jobHandle1, 1U, 2U);
    callback->dataFramesSent(jobHandle3, 1U, 4U);
    // expect released from context
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message1),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message3),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);

    // shutdown
    ASSERT_TRUE(messageTransmitterBlockPool.empty());
    cut.shutdown();
}

// The messages are first initialized and all have the same timeout, TX Timeout
// 1. The last message should get 0 flow control, which will just send everything. This will get
// processed before any sorting happens.
// 2. Last of the remaining 3 gets a flowControl with min separation time, sorting happens and it
// should be first in line. Trigger a send, nothing will happen, but pass the time of that
// flowControl and only that should expire and send the previously triggered send.
// 3. Finish the transmit of the first of the remaining 2 messages, causing the timeout to be after
// the the second transmit TX timeout. Then it's sorted and the second message, now first after
// sorting, will expire once it's time has passed..
// 4. Make sure the last message expires at on flowcontrol timeout time after the 3rd step.
TEST_F(DoCanTransmitterTest, testMultipleDifferentTimeoutsWithDifferentExpiry)
{
    nowUs                          = 0;
    constexpr uint8_t MESSAGE_SIZE = 15;
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), MESSAGE_SIZE> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();
    constexpr uint8_t NO_OF_MESSAGES
        = 4; //   1 Flowcontrol timeout, 1 TX timeout, 1 min separation time, 1 succeeding

    constexpr uint8_t FLOWCONTROL_TIMEOUT   = 0;
    constexpr uint8_t TX_TIMEOUT            = 1;
    constexpr uint8_t MINSEPARATION_TIMEOUT = 2;
    constexpr uint8_t SUCCESS               = 3;
    uint8_t data[NO_OF_MESSAGES][MESSAGE_SIZE];
    TransportMessage transportMessages[NO_OF_MESSAGES];
    JobHandle jobHandles[NO_OF_MESSAGES];
    IDoCanDataFrameTransmitterCallback<DataLinkLayer>* callback = nullptr;
    DataLinkLayer::AddressPairType addrPair[NO_OF_MESSAGES];
    DoCanTransportAddressPair transportPair[NO_OF_MESSAGES];
    _context.handleExecute();
    for (uint8_t i = 0; i < NO_OF_MESSAGES; i++)
    {
        nowUs = i;
        for (uint8_t j = 0; j < MESSAGE_SIZE; j++)
        {
            data[i][j] = i + j;
        }
        addrPair[i]      = DataLinkLayer::AddressPairType(0x1234 + i, 0x5678 + i);
        transportPair[i] = DoCanTransportAddressPair(0x45 + i, 0x54 + i);
        initMessage(transportMessages[i], transportPair[i], addrPair[i], data[i]);
        ASSERT_EQ(
            ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
            cut.send(transportMessages[i], &_processedListenerMock));
        EXPECT_CALL(
            _dataFrameTransmitterMock,
            startSendDataFrames(
                _,
                _,
                _,
                addrPair[i].getTransmissionAddress(),
                0U,
                1U,
                7U,
                ElementsAreArray(data[i])))
            .WillOnce(DoAll(
                WithArg<1>(SaveRef<0>(&callback)),
                SaveArg<2>(&jobHandles[i]),
                Return(SendResult::QUEUED)));
        _context.execute();
        cut.cyclicTask(nowUs);
        Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    }

    // Success and expiration in opposite order of adding to test sorting of messages.

    // 1. Instant success of the last
    callback->dataFramesSent(jobHandles[SUCCESS], 1U, 7U);
    // Expect all immediate send after flowcontrol
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandles[SUCCESS],
            addrPair[SUCCESS].getTransmissionAddress(),
            1U,
            3U,
            7U,
            ElementsAreArray(::etl::span<uint8_t const>(data[SUCCESS]).subspan(7U))))
        .WillOnce(Return(SendResult::QUEUED));
    cut.flowControlFrameReceived(addrPair[SUCCESS].getReceptionAddress(), FlowStatus::CTS, 0U, 0U);
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandles[SUCCESS],
            addrPair[SUCCESS].getTransmissionAddress(),
            2U,
            3U,
            7U,
            ElementsAreArray(::etl::span<uint8_t const>(data[SUCCESS]).subspan(12U))))
        .WillOnce(Return(SendResult::QUEUED));
    callback->dataFramesSent(jobHandles[SUCCESS], 1U, 5U);
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    callback->dataFramesSent(jobHandles[SUCCESS], 1U, 2U);

    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(transportMessages[SUCCESS]),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_processedListenerMock);

    cut.cyclicTask(nowUs);
    nowUs += 1000; // 1 ms
    cut.cyclicTask(nowUs);

    // 2. Min separation time
    callback->dataFramesSent(jobHandles[MINSEPARATION_TIMEOUT], 1U, 7U);
    // Expect one immediate send after flowcontrol
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandles[MINSEPARATION_TIMEOUT],
            addrPair[MINSEPARATION_TIMEOUT].getTransmissionAddress(),
            1U,
            2U,
            7U,
            ElementsAreArray(::etl::span<uint8_t const>(data[MINSEPARATION_TIMEOUT]).subspan(7U))))
        .WillOnce(Return(SendResult::QUEUED));

    cut.flowControlFrameReceived(
        addrPair[MINSEPARATION_TIMEOUT].getReceptionAddress(), FlowStatus::CTS, 0U, 8U);

    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    EXPECT_CALL(_tickGeneratorMock, tickNeeded());
    callback->dataFramesSent(jobHandles[MINSEPARATION_TIMEOUT], 1U, 5U);

    // Expect no immediate send after frames sent
    cut.cyclicTask(nowUs); // This call will sort the timers
    nowUs += 7000;
    cut.cyclicTask(nowUs); // This call should check only minSeparation time expiry
    nowUs += 999;
    cut.cyclicTask(nowUs);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandles[MINSEPARATION_TIMEOUT],
            addrPair[MINSEPARATION_TIMEOUT].getTransmissionAddress(),
            2U,
            3U,
            7U,
            ElementsAreArray(::etl::span<uint8_t const>(data[MINSEPARATION_TIMEOUT]).subspan(12U))))
        .WillOnce(Return(SendResult::QUEUED));
    nowUs += 1;
    cut.cyclicTask(nowUs);
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);

    callback->dataFramesSent(jobHandles[MINSEPARATION_TIMEOUT], 1U, 2U);
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(transportMessages[MINSEPARATION_TIMEOUT]),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_processedListenerMock);

    // 4. FlowControl timeout setup
    nowUs += 1000;
    auto const flowControlTimeoutTime = nowUs + (waitFlowControlTimeout * 1000);
    callback->dataFramesSent(jobHandles[FLOWCONTROL_TIMEOUT], 1U, 7U);

    // 3. TX Timeout
    cut.cyclicTask(nowUs); // This should sort and make sure FLOWCONTROL_TIMEOUT is last, only
                           // checking expiry on TX_TIMEOUT
    nowUs = waitTxCallbackTimeout * 1000 - 1 + TX_TIMEOUT;
    cut.cyclicTask(nowUs);
    expectLog(LEVEL_WARN);
    EXPECT_CALL(
        _dataFrameTransmitterMock, cancelSendDataFrames(Ref(*callback), jobHandles[TX_TIMEOUT]));
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(transportMessages[TX_TIMEOUT]),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR));

    nowUs += 1;
    cut.cyclicTask(nowUs);
    Mock::VerifyAndClearExpectations(&_processedListenerMock);

    // 4. FlowControl timeout
    nowUs = flowControlTimeoutTime - 1;
    cut.cyclicTask(nowUs);
    expectLog(LEVEL_WARN);
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(transportMessages[FLOWCONTROL_TIMEOUT]),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR_TIMEOUT));
    nowUs += 1;
    cut.cyclicTask(nowUs);
    Mock::VerifyAndClearExpectations(&_processedListenerMock);

    // shutdown
    ASSERT_TRUE(messageTransmitterBlockPool.empty());
    cut.shutdown();
}

TEST_F(DoCanTransmitterTest, testSendMessageFailsOnSendError)
{
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 5U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    // content
    uint8_t data[] = {0xab, 0xcd, 0xef, 0x19, 0x28};
    TransportMessage message;
    auto const addrPair      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message, transportPair, addrPair, data);
    // try sending the segmented message and expect error
    _context.handleExecute();
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message, &_processedListenerMock));
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _, _, _, addrPair.getTransmissionAddress(), 0U, 1U, 0U, ElementsAreArray(data)))
        .WillOnce(Return(SendResult::FAILED));
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message), ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);

    // shutdown
    ASSERT_TRUE(messageTransmitterBlockPool.empty());
    cut.shutdown();
}

TEST_F(DoCanTransmitterTest, testSendEmptyMessage)
{
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 5U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    // No content
    uint8_t data[] = {};
    TransportMessage message;
    auto const addrPair      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair = DoCanTransportAddressPair(0x45, 0x54);
    message.init(data, sizeof(data));
    message.setPayloadLength(sizeof(data));
    message.setSourceId(transportPair.getSourceId());
    message.setTargetId(transportPair.getTargetId());

    EXPECT_CALL(_addressConverterMock, getTransmissionParameters(transportPair, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(addrPair), Return(&_codec)));

    // try sending the empty message and expect error
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_GENERAL_ERROR,
        cut.send(message, &_processedListenerMock));
}

TEST_F(DoCanTransmitterTest, testSendTooBigMessage)
{
    StrictMock<DoCanDataFrameTransmitterMock<SmallFrameIndexDataLinkLayer>>
        smallDataFrameTransmitterMock;
    StrictMock<DoCanAddressConverterMock<SmallFrameIndexDataLinkLayer>> smallAddressConverterMock;
    DoCanFrameCodec<SmallFrameIndexDataLinkLayer> smallFrameCodec(
        DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, _mapper);

    using T = ::docan::DoCanMessageTransmitter<SmallFrameIndexDataLinkLayer>;
    ::etl::generic_pool<sizeof(T), alignof(T), 5U> messageTransmitterBlockPool;
    DoCanTransmitter<SmallFrameIndexDataLinkLayer> cut(
        _busId,
        _context,
        smallDataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        smallAddressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    // Bigger than DataLinkLayer::FrameIndexType size
    uint8_t
        data[::std::numeric_limits<SmallFrameIndexDataLinkLayer::FrameIndexType>::max() * 8 + 1];
    ::etl::mem_set(data, sizeof(data), static_cast<uint8_t>(0xA5));

    TransportMessage message;
    auto const addrPair      = SmallFrameIndexDataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair = DoCanTransportAddressPair(0x45, 0x54);
    message.init(data, sizeof(data));
    message.setPayloadLength(sizeof(data));
    message.increaseValidBytes(sizeof(data));
    message.setSourceId(transportPair.getSourceId());
    message.setTargetId(transportPair.getTargetId());
    EXPECT_CALL(smallAddressConverterMock, getTransmissionParameters(transportPair, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(addrPair), Return(&smallFrameCodec)));

    // try sending the message and expect error
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_GENERAL_ERROR,
        cut.send(message, &_processedListenerMock));
}

TEST_F(DoCanTransmitterTest, testSendIncompleteMessage)
{
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 5U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    // content
    uint8_t data[] = {0xab, 0xcd, 0xef, 0x19, 0x28};
    TransportMessage message;
    auto const addrPair      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair = DoCanTransportAddressPair(0x45, 0x54);
    message.init(data, sizeof(data));
    message.setPayloadLength(sizeof(data));
    message.setSourceId(transportPair.getSourceId());
    message.setTargetId(transportPair.getTargetId());
    EXPECT_CALL(_addressConverterMock, getTransmissionParameters(transportPair, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(addrPair), Return(&_codec)));

    // try sending the segmented message and expect error
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_MESSAGE_INCOMPLETE,
        cut.send(message, &_processedListenerMock));
}

TEST_F(DoCanTransmitterTest, testSendSecondSegmentedMessageForSameTransportAddressPair)
{
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 5U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    // content
    uint8_t data[] = {0xab, 0xcd, 0xef, 0x19, 0x28, 0x35, 0x89, 0x11};
    // send first segmented message
    TransportMessage message1;
    auto const addrPair1      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair1 = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message1, transportPair1, addrPair1, data);
    _context.handleExecute();
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message1, &_processedListenerMock));
    // sending second segmented message and expect error
    TransportMessage message2;
    auto const addrPair2      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair2 = DoCanTransportAddressPair(0x46, 0x64);
    initMessage(message2, transportPair2, addrPair2, data);
    expectLog(LEVEL_WARN);
    EXPECT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_SEND_FAIL,
        cut.send(message2, &_processedListenerMock));

    // shut down => cancel sends
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message1), ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR));
    cut.shutdown();
    ASSERT_TRUE(messageTransmitterBlockPool.empty());
}

TEST_F(DoCanTransmitterTest, testFillUpSendQueue)
{
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 2U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    // content
    uint8_t data[] = {0xab, 0xcd, 0xef, 0x19};
    // send first segmented message
    TransportMessage message1;
    auto const addrPair1      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair1 = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message1, transportPair1, addrPair1, data);
    _context.handleExecute();
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message1, &_processedListenerMock));
    // sending second segmented message and expect error
    TransportMessage message2;
    auto const addrPair2      = DataLinkLayer::AddressPairType(0x4321, 0x8765);
    auto const transportPair2 = DoCanTransportAddressPair(0x35, 0x53);
    initMessage(message2, transportPair2, addrPair2, data);
    _context.handleExecute();
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message2, &_processedListenerMock));
    // sending third segmented message is expected to fail
    TransportMessage message3;
    auto const addrPair3      = DataLinkLayer::AddressPairType(0xABCD, 0xDCBA);
    auto const transportPair3 = DoCanTransportAddressPair(0x48, 0x84);
    initMessage(message3, transportPair3, addrPair3, data);
    expectLog(LEVEL_WARN, addrPair3.getReceptionAddress());
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_QUEUE_FULL,
        cut.send(message3, &_processedListenerMock));

    // shut down => cancel sends
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message1), ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR));
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message2), ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR));
    cut.shutdown();
    ASSERT_TRUE(messageTransmitterBlockPool.empty());
}

TEST_F(DoCanTransmitterTest, testDataLinkQueueFull)
{
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 5U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    // content
    uint8_t data[] = {0xab, 0xcd, 0xef, 0x19, 0x28};
    TransportMessage message;
    auto const addrPair      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message, transportPair, addrPair, data);
    // send the message...
    _context.handleExecute();
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message, &_processedListenerMock));
    // ... and let call fail from context
    IDoCanDataFrameTransmitterCallback<DataLinkLayer>* callback = nullptr;
    JobHandle jobHandle(0x4f, 0x99);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _, _, _, addrPair.getTransmissionAddress(), 0U, 1U, 0U, ElementsAreArray(data)))
        .WillOnce(DoAll(
            WithArg<1>(SaveRef<0>(&callback)), SaveArg<2>(&jobHandle), Return(SendResult::FULL)));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // expect retry from within cyclic task
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _,
            Ref(*callback),
            jobHandle,
            addrPair.getTransmissionAddress(),
            0U,
            1U,
            0U,
            ElementsAreArray(data)))
        .WillOnce(Return(SendResult::QUEUED_FULL));
    cut.cyclicTask(nowUs);
    // processing done after send result
    callback->dataFramesSent(jobHandle, 1U, sizeof(data));
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));
    _context.execute();
}

TEST_F(DoCanTransmitterTest, flowControlFrameTimeout)
{
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 5U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);

    cut.init();
    uint8_t data[] = {
        0xab, 0xcd, 0xef, 0x19, 0x28, 0x98, 0xa1, 0x45, 0x11, 0x22, 0x33, 0x44, 0x55, 0x67, 0x9e};
    TransportMessage message;
    auto const addrPair      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message, transportPair, addrPair, data);
    // send the segmented message
    _context.handleExecute();
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message, &_processedListenerMock));
    // expect first send when looping
    JobHandle jobHandle(0x1f, 0x99);
    IDoCanDataFrameTransmitterCallback<DataLinkLayer>* callback = nullptr;
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(_, _, _, 0x5678, 0U, 1U, 7U, ElementsAreArray(data)))
        .WillOnce(DoAll(
            WithArg<1>(SaveRef<0>(&callback)),
            SaveArg<2>(&jobHandle),
            Return(SendResult::QUEUED_FULL)));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // frames sent
    callback->dataFramesSent(jobHandle, 1U, 6U);

    // Ensure the flow control timeout is processed at exactly the specified timeout time
    nowUs = 399999;
    cut.cyclicTask(nowUs);
    nowUs = 400000;
    // expect cancel of send job and failed processing from flow control timeout
    expectLog(LEVEL_WARN);
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR_TIMEOUT));
    cut.cyclicTask(nowUs);
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
}

TEST_F(DoCanTransmitterTest, testSendSecondMessageAfterFirstMessageTransmissionTimeout)
{
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 5U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();

    uint8_t data[] = {0xab, 0xcd, 0xef, 0x19, 0x28};
    TransportMessage message;
    auto const addrPair      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message, transportPair, addrPair, data);
    // send the message
    _context.handleExecute();
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message, &_processedListenerMock));
    // loop and expect frame written
    IDoCanDataFrameTransmitterCallback<DataLinkLayer>* callback = nullptr;
    JobHandle jobHandle(0x4f, 0x19);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _, _, _, addrPair.getTransmissionAddress(), 0U, 1U, 0U, ElementsAreArray(data)))
        .WillOnce(DoAll(
            WithArg<1>(SaveRef<0>(&callback)),
            SaveArg<2>(&jobHandle),
            Return(SendResult::QUEUED_FULL)));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    cut.cyclicTask(nowUs);
    cut.cyclicTask(nowUs);
    // Ensure the timeout behavior doesn't happen until _exactly_ 300ms
    nowUs = 299999;
    cut.cyclicTask(nowUs);
    nowUs = 300000;
    // expect cancel of send job and failed processing
    expectLog(LEVEL_WARN);
    EXPECT_CALL(_dataFrameTransmitterMock, cancelSendDataFrames(Ref(*callback), jobHandle));
    EXPECT_CALL(
        _processedListenerMock,
        transportMessageProcessed(
            Ref(message), ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR));
    cut.cyclicTask(nowUs);
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);
    // processing done after send result
    _context.execute();
}

TEST_F(DoCanTransmitterTest, testCanFrameSentIgnoredIfNoPendingSender)
{
    ::etl::generic_pool<sizeof(ItemT), alignof(ItemT), 5U> messageTransmitterBlockPool;
    DoCanTransmitter<DataLinkLayer> cut(
        _busId,
        _context,
        _dataFrameTransmitterMock,
        _tickGeneratorMock,
        messageTransmitterBlockPool,
        _addressConverterMock,
        _parameters,
        _loggerComponent);
    cut.init();
    // send a message to receive callback
    uint8_t data[] = {0xab, 0xcd, 0xef, 0x19, 0x28};
    TransportMessage message;
    auto const addrPair      = DataLinkLayer::AddressPairType(0x1234, 0x5678);
    auto const transportPair = DoCanTransportAddressPair(0x45, 0x54);
    initMessage(message, transportPair, addrPair, data);
    // send the message
    _context.handleExecute();
    ASSERT_EQ(
        ::transport::AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message, &_processedListenerMock));
    // loop and expect frame written
    IDoCanDataFrameTransmitterCallback<DataLinkLayer>* callback = nullptr;
    JobHandle jobHandle(0x15, 0x19);
    EXPECT_CALL(
        _dataFrameTransmitterMock,
        startSendDataFrames(
            _, _, _, addrPair.getTransmissionAddress(), 0U, 1U, 0U, ElementsAreArray(data)))
        .WillOnce(DoAll(
            WithArg<1>(SaveRef<0>(&callback)),
            SaveArg<2>(&jobHandle),
            Return(SendResult::QUEUED_FULL)));
    _context.execute();
    Mock::VerifyAndClearExpectations(&_dataFrameTransmitterMock);

    callback->dataFramesSent(jobHandle, 1U, 5U);
}

void DoCanTransmitterTest::expectLog(Level const level)
{
    EXPECT_CALL(_componentMappingMock, isEnabled(_loggerComponent, level)).WillOnce(Return(false));
}

void DoCanTransmitterTest::expectLog(Level const level, uint32_t address)
{
    expectLog(level);
    EXPECT_CALL(_addressConverterMock, formatDataLinkAddress(address, _)).WillOnce(Return("abc"));
}

void DoCanTransmitterTest::expectLog(Level const level, char const* logMessage)
{
    EXPECT_CALL(_componentMappingMock, isEnabled(_loggerComponent, level)).WillOnce(Return(true));
    EXPECT_CALL(_componentMappingMock, getLevelInfo(level)).Times(1);
    EXPECT_CALL(_componentMappingMock, getComponentInfo(_)).Times(1);
    EXPECT_CALL(_loggerOutputMock, logOutput(_, _, logMessage, _)).Times(1);
}

} // anonymous namespace
