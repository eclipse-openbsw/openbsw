// Copyright 2024 Accenture.

#include "docan/addressing/DoCanNormalAddressing.h"
#include "docan/addressing/DoCanNormalAddressingFilter.h"
#include "docan/can/DoCanPhysicalCanTransceiver.h"
#include "docan/common/DoCanConstants.h"
#include "docan/common/DoCanParameters.h"
#include "docan/datalink/DoCanDefaultFrameSizeMapper.h"
#include "docan/datalink/DoCanFrameCodec.h"
#include "docan/datalink/DoCanFrameCodecConfigPresets.h"
#include "docan/datalink/IDoCanFrameReceiver.h"
#include "docan/transmitter/IDoCanTickGenerator.h"
#include "docan/transport/DoCanTransportLayerConfig.h"
#include "docan/transport/DoCanTransportLayerContainer.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <benchmark/benchmark.h>
#include <can/canframes/CanId.h>
#include <can/transceiver/AbstractCANTransceiver.h>
#include <can/transceiver/ICanTransceiver.h>
#include <transport/BufferedTransportMessage.h>
#include <transport/ITransportMessageProcessedListener.h>
#include <transport/TransportMessage.h>

#include <estd/vector.h>

#include <gtest/gtest.h>

struct TransportMessageProcessedListener : public ::transport::ITransportMessageProcessedListener
{
    TransportMessageProcessedListener(uint32_t const& nowUs, bool& sending)
    : nowUs(nowUs), sending(sending), sentFrameCount(0)
    {}

    void transportMessageProcessed(
        ::transport::TransportMessage& message,
        ::transport::ITransportMessageProcessedListener::ProcessingResult result) override
    {
        ASSERT_EQ(
            result,
            ::transport::ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
        ++sentFrameCount;
        sending = false;
    }

    uint32_t const& nowUs;
    bool& sending;
    uint64_t sentFrameCount;
};

struct CanTransceiver : public ::can::AbstractCANTransceiver
{
    CanTransceiver(uint8_t busId) : AbstractCANTransceiver(busId) {}

    virtual ~CanTransceiver() = default;

    ErrorCode init() override { return ::can::ICanTransceiver::ErrorCode::CAN_ERR_OK; }

    void shutdown() override {}

    ErrorCode open(::can::CANFrame const& frame) override
    {
        return ::can::ICanTransceiver::ErrorCode::CAN_ERR_OK;
    }

    ErrorCode open() override { return ::can::ICanTransceiver::ErrorCode::CAN_ERR_OK; }

    ErrorCode close() override { return ::can::ICanTransceiver::ErrorCode::CAN_ERR_OK; }

    ErrorCode mute() override { return ::can::ICanTransceiver::ErrorCode::CAN_ERR_OK; }

    ErrorCode unmute() override { return ::can::ICanTransceiver::ErrorCode::CAN_ERR_OK; }

    uint32_t getBaudrate() const override { return 0; }

    uint16_t getHwQueueTimeout() const override { return 0; }

    ErrorCode write(::can::CANFrame const& frame) override
    {
        return ::can::ICanTransceiver::ErrorCode::CAN_ERR_OK;
    }

    ErrorCode write(::can::CANFrame const& frame, ::can::ICANFrameSentListener& listener) override
    {
        return ::can::ICanTransceiver::ErrorCode::CAN_ERR_OK;
    }
};

struct TickGeneratorAdapter : public ::docan::IDoCanTickGenerator
{
    void tickNeeded() override {}
};

static uint32_t nowUs = 0;
// I rather despise the combination of singleton mocks to handle free functions and google benchmark
// (which keeps multiple instances of benchmarking fixtures alive simultaneously), as it forces
// these singletons to stand on their own and complicates their initialization logic
// AsyncMock must only be initialized once, so control initialization with a boolean. If this code
// is every multi threaded it'll have to become an atomic boolean
alignas(::async::AsyncMock) uint8_t asyncMockMem[sizeof(::async::AsyncMock)];
std::once_flag asyncMockInitialized;

uint32_t nowUsFunc() { return nowUs; }

static uint16_t const ALLOCATE_TIMEOUT       = 800;
static uint16_t const RX_TIMEOUT             = 1000U;
static uint16_t const TX_CALLBACK_TIMEOUT    = 1000U;
static uint16_t const FLOW_CONTROL_TIMEOUT   = 1000U;
static uint8_t const ALLOCATE_RETRY_COUNT    = 15U;
static uint8_t const FLOW_CONTROL_WAIT_COUNT = 15U;
static uint16_t const MIN_SEPARATION_TIME    = 20U; // set to 0?
static uint8_t const BLOCK_SIZE              = 0U;

static constexpr uint8_t NUM_CAN_TRANSPORT_ISO_LAYER = 5;

using AddressingCodec   = ::docan::DoCanNormalAddressing<>;
using DataLinkLayerType = AddressingCodec::DataLinkLayerType;
using DoCanIsoLayer     = ::docan::DoCanTransportLayer<DataLinkLayerType>;
using FrameCodecType    = ::docan::DoCanFrameCodec<DataLinkLayerType>;
using MapperType        = ::docan::DoCanDefaultFrameSizeMapper<DataLinkLayerType::FrameSizeType>;

struct TransmissionBenchmark : public ::benchmark::Fixture
{
    TransmissionBenchmark()
    {
        std::call_once(asyncMockInitialized, []() { new (asyncMockMem)::async::AsyncMock(); });
    }

    AddressingCodec _doCanCodecClassic{};
    ::docan::DoCanParameters _doCanParameters{
        ::estd::function<uint32_t()>::create<&nowUsFunc>(),
        ALLOCATE_TIMEOUT,
        RX_TIMEOUT,
        TX_CALLBACK_TIMEOUT,
        FLOW_CONTROL_TIMEOUT,
        ALLOCATE_RETRY_COUNT,
        FLOW_CONTROL_WAIT_COUNT,
        MIN_SEPARATION_TIME,
        BLOCK_SIZE};

    static constexpr ::docan::DoCanNormalAddressingFilterAddressEntry<DataLinkLayerType>
        doCanMappingEntries[] = {
            /*canReceptionId*/ /*canTransmissionId*/ /*transportSourceId*/ /*transportTargetId*/
            {::can::CanId::Base<0x415>::value,
             ::can::CanId::Base<0x414>::value,
             0x11U,
             0x10U,
             0, // normal codec idx
             0} // normal codec idx
        };

    MapperType const mapper{};
    FrameCodecType const codecClassic{
        ::docan::DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper};
    FrameCodecType const codecFd{::docan::DoCanFrameCodecConfigPresets::OPTIMIZED_FD, mapper};
    FrameCodecType const* codecEntries[2] = {&codecClassic, &codecFd};

    ::docan::DoCanNormalAddressingFilter<DataLinkLayerType> _doCanAddressingFilter{
        ::estd::make_slice(doCanMappingEntries), ::estd::make_slice(codecEntries)};

    ::common::busid::internal::Id<uint8_t, 0U> id;

    TickGeneratorAdapter _doCanTickGenerator;

    ::transport::BufferedTransportMessage<10> transportMessage;
    uint8_t data[10];

    void SetUp(::benchmark::State&) override
    {
        ::testing::Mock::AllowLeak(&asyncMockMem);
        nowUs = 0;
        _context.handleExecute();

        for (size_t idx = 0; idx < sizeof(data); ++idx)
        {
            data[idx] = idx % 256;
        }
        transportMessage.setSourceId(0x10U);
        transportMessage.setTargetId(0x11U);
        transportMessage.append(data, sizeof(data));
        transportMessage.setPayloadLength(sizeof(data));
    }

    ::docan::declare::DoCanTransportLayerConfig<DataLinkLayerType, 80U, 15U, 64U> _doCanConfig{
        _doCanParameters};
    ::async::TestContext _context{1};

    CanTransceiver canTransceiver{id};

    ::estd::declare::vector<DoCanIsoLayer, NUM_CAN_TRANSPORT_ISO_LAYER> _doCanIsoLayers;
    ::estd::declare::
        vector<::docan::DoCanPhysicalCanTransceiver<AddressingCodec>, NUM_CAN_TRANSPORT_ISO_LAYER>
            _doCanPhysicalTransceivers;
    ::can::ICANFrameSentListener* canFrameSentListener{nullptr};

    void initializeStack()
    {
        new (&_doCanConfig)::docan::declare::
            DoCanTransportLayerConfig<DataLinkLayerType, 80U, 15U, 64U>(_doCanParameters);

        auto shutdownCallback = [](::transport::AbstractTransportLayer&) {};

        for (auto&& layer : _doCanIsoLayers)
        {
            layer.shutdown(::estd::make_function(shutdownCallback));
            _context.execute();
        }

        _doCanIsoLayers.clear();
        _doCanPhysicalTransceivers.clear();

        ::docan::DoCanPhysicalCanTransceiver<AddressingCodec>& doCanTransceiver
            = _doCanPhysicalTransceivers.emplace_back().construct(
                ::estd::by_ref(canTransceiver),
                ::estd::by_ref(_doCanAddressingFilter),
                _doCanAddressingFilter,
                _doCanCodecClassic);

        canFrameSentListener = &doCanTransceiver;

        _doCanIsoLayers.emplace_back().construct(
            id,
            _context,
            _doCanAddressingFilter,
            doCanTransceiver,
            _doCanTickGenerator,
            _doCanConfig,
            0U);

        for (auto&& layer : _doCanIsoLayers)
        {
            layer.init();
        }
    }
};

BENCHMARK_F(TransmissionBenchmark, StartSend)(benchmark::State& state)
{
    for (auto _ : state)
    {
        initializeStack();
        ASSERT_EQ(
            _doCanIsoLayers[0].send(transportMessage, nullptr),
            ::transport::AbstractTransportLayer::ErrorCode::TP_OK);
    }
}

BENCHMARK_F(TransmissionBenchmark, StartSendAndLoop)(benchmark::State& state)
{
    for (auto _ : state)
    {
        initializeStack();
        ASSERT_EQ(
            _doCanIsoLayers[0].send(transportMessage, nullptr),
            ::transport::AbstractTransportLayer::ErrorCode::TP_OK);
        _context.execute();
    }
}

BENCHMARK_F(TransmissionBenchmark, SendFirstFrame)(benchmark::State& state)
{
    for (auto _ : state)
    {
        initializeStack();

        ASSERT_EQ(
            _doCanIsoLayers[0].send(transportMessage, nullptr),
            ::transport::AbstractTransportLayer::ErrorCode::TP_OK);
        _context.execute();
        canFrameSentListener->canFrameSent({});
    }
}

BENCHMARK_F(TransmissionBenchmark, SendFirstFrameAndLoop)(benchmark::State& state)
{
    for (auto _ : state)
    {
        initializeStack();

        ASSERT_EQ(
            _doCanIsoLayers[0].send(transportMessage, nullptr),
            ::transport::AbstractTransportLayer::ErrorCode::TP_OK);
        _context.execute();
        canFrameSentListener->canFrameSent({});
        _context.execute();
    }
}

BENCHMARK_F(TransmissionBenchmark, SendFlowControl)(benchmark::State& state)
{
    for (auto _ : state)
    {
        initializeStack();
        ::docan::IDoCanFrameReceiver<DataLinkLayerType>* canFrameReceiver(&_doCanIsoLayers[0]);

        ASSERT_EQ(
            _doCanIsoLayers[0].send(transportMessage, nullptr),
            ::transport::AbstractTransportLayer::ErrorCode::TP_OK);
        _context.execute();
        canFrameSentListener->canFrameSent({});
        _context.execute();
        canFrameReceiver->flowControlFrameReceived(0x415, ::docan::FlowStatus::CTS, 0U, 0U);
    }
}

BENCHMARK_F(TransmissionBenchmark, SendFirstConsecutiveFrame)(benchmark::State& state)
{
    for (auto _ : state)
    {
        initializeStack();
        ::docan::IDoCanFrameReceiver<DataLinkLayerType>* canFrameReceiver(&_doCanIsoLayers[0]);

        ASSERT_EQ(
            _doCanIsoLayers[0].send(transportMessage, nullptr),
            ::transport::AbstractTransportLayer::ErrorCode::TP_OK);
        _context.execute();
        canFrameSentListener->canFrameSent({});
        _context.execute();
        canFrameReceiver->flowControlFrameReceived(0x415, ::docan::FlowStatus::CTS, 0U, 0U);

        canFrameSentListener->canFrameSent({});
        _context.execute();
    }
}

template<size_t MessageSize>
void TransmissionFullSegmentedMessage(benchmark::State& state)
{
    nowUs = 0;
    ::async::TestContext _context{1};

    AddressingCodec _doCanCodecClassic;
    ::docan::DoCanParameters _doCanParameters{
        ::estd::function<uint32_t()>::create<&nowUsFunc>(),
        ALLOCATE_TIMEOUT,
        RX_TIMEOUT,
        TX_CALLBACK_TIMEOUT,
        FLOW_CONTROL_TIMEOUT,
        ALLOCATE_RETRY_COUNT,
        FLOW_CONTROL_WAIT_COUNT,
        MIN_SEPARATION_TIME,
        BLOCK_SIZE};

    constexpr ::docan::DoCanNormalAddressingFilterAddressEntry<DataLinkLayerType>
        doCanMappingEntries[] = {
            /*canReceptionId*/ /*canTransmissionId*/ /*transportSourceId*/ /*transportTargetId*/
            {::can::CanId::Base<0x415>::value,
             ::can::CanId::Base<0x414>::value,
             0x11U,
             0x10U,
             0, // normal codec idx
             0} // normal codec idx
        };

    using FrameCodecType = ::docan::DoCanFrameCodec<DataLinkLayerType>;
    using MapperType     = ::docan::DoCanDefaultFrameSizeMapper<DataLinkLayerType::FrameSizeType>;

    MapperType const mapper;
    FrameCodecType const codecClassic(
        ::docan::DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    FrameCodecType const codecFd(::docan::DoCanFrameCodecConfigPresets::OPTIMIZED_FD, mapper);
    FrameCodecType const* codecEntries[2] = {&codecClassic, &codecFd};

    ::docan::DoCanNormalAddressingFilter<DataLinkLayerType> _doCanAddressingFilter{
        ::estd::make_slice(doCanMappingEntries), ::estd::make_slice(codecEntries)};

    ::docan::declare::DoCanTransportLayerConfig<DataLinkLayerType, 80U, 15U, 64U> _doCanConfig(
        _doCanParameters);
    ::estd::declare::vector<DoCanIsoLayer, NUM_CAN_TRANSPORT_ISO_LAYER> _doCanIsoLayers;
    ::docan::DoCanTransportLayerContainer<DataLinkLayerType> _doCanIsoLayerContainer(
        _doCanIsoLayers);
    ::estd::declare::
        vector<::docan::DoCanPhysicalCanTransceiver<AddressingCodec>, NUM_CAN_TRANSPORT_ISO_LAYER>
            _doCanPhysicalTransceivers;

    ::common::busid::internal::Id<uint8_t, 0U> id;

    CanTransceiver canTransceiver(id);

    TickGeneratorAdapter _doCanTickGenerator;

    ::docan::DoCanPhysicalCanTransceiver<AddressingCodec>& doCanTransceiver
        = _doCanPhysicalTransceivers.emplace_back().construct(
            ::estd::by_ref(canTransceiver),
            ::estd::by_ref(_doCanAddressingFilter),
            _doCanAddressingFilter,
            _doCanCodecClassic);

    ::can::ICANFrameSentListener* canFrameSentListener(&doCanTransceiver);

    _doCanIsoLayers.emplace_back().construct(
        id,
        _context,
        _doCanAddressingFilter,
        doCanTransceiver,
        _doCanTickGenerator,
        _doCanConfig,
        0U);

    ::docan::IDoCanFrameReceiver<DataLinkLayerType>* canFrameReceiver(&_doCanIsoLayers[0]);

    _doCanIsoLayerContainer.init();

    ::transport::BufferedTransportMessage<MessageSize> transportMessage;
    uint8_t data[MessageSize];
    for (size_t idx = 0; idx < sizeof(data); ++idx)
    {
        data[idx] = idx % 256;
    }
    transportMessage.setSourceId(0x10U);
    transportMessage.setTargetId(0x11U);
    transportMessage.append(data, sizeof(data));
    transportMessage.setPayloadLength(sizeof(data));

    // can use in the future to set up for another send immediately after the current one
    // finishes
    bool sending = false;
    TransportMessageProcessedListener tpMessageProcessedListener(nowUs, sending);

    _context.handleExecute();

    for (auto _ : state)
    {
        ++nowUs;
        auto& doCanIsoTransportLayer = _doCanIsoLayers[0];
        ASSERT_EQ(
            doCanIsoTransportLayer.send(transportMessage, &tpMessageProcessedListener),
            ::transport::AbstractTransportLayer::ErrorCode::TP_OK);
        sending = true;
        _context.execute();
        canFrameSentListener->canFrameSent({});
        _context.execute();
        canFrameReceiver->flowControlFrameReceived(0x415, ::docan::FlowStatus::CTS, 0U, 0U);

        while (sending)
        {
            canFrameSentListener->canFrameSent({});
            _context.execute();
        }
    }
}

BENCHMARK_TEMPLATE(TransmissionFullSegmentedMessage, 100);
BENCHMARK_TEMPLATE(TransmissionFullSegmentedMessage, 1000);
BENCHMARK_TEMPLATE(TransmissionFullSegmentedMessage, 10000);

template<size_t MessageSize, uint16_t NoOfMessages>
void TransmissionMultipleTransportLayersFullSegmentedMessages(benchmark::State& state)
{
    nowUs = 0;
    ::async::TestContext _context{1};

    AddressingCodec _doCanCodecClassic;
    ::docan::DoCanParameters _doCanParameters{
        ::estd::function<uint32_t()>::create<&nowUsFunc>(),
        ALLOCATE_TIMEOUT,
        RX_TIMEOUT,
        TX_CALLBACK_TIMEOUT,
        FLOW_CONTROL_TIMEOUT,
        ALLOCATE_RETRY_COUNT,
        FLOW_CONTROL_WAIT_COUNT,
        MIN_SEPARATION_TIME,
        BLOCK_SIZE};

    using FrameCodecType = ::docan::DoCanFrameCodec<DataLinkLayerType>;
    using MapperType     = ::docan::DoCanDefaultFrameSizeMapper<DataLinkLayerType::FrameSizeType>;

    MapperType const mapper;
    FrameCodecType const codecClassic(
        ::docan::DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    FrameCodecType const codecFd(::docan::DoCanFrameCodecConfigPresets::OPTIMIZED_FD, mapper);
    FrameCodecType const* codecEntries[2] = {&codecClassic, &codecFd};

    ::estd::declare::
        vector<::docan::DoCanNormalAddressingFilterAddressEntry<DataLinkLayerType>, NoOfMessages>
            doCanMappingEntries;
    for (uint16_t messageIndex = 0; messageIndex < NoOfMessages; ++messageIndex)
    {
        doCanMappingEntries.emplace_back().construct(
            ::docan::DoCanNormalAddressingFilterAddressEntry<DataLinkLayerType>(
                {messageIndex,
                 static_cast<uint16_t>(messageIndex + 1U),
                 static_cast<uint16_t>(messageIndex + 2U),
                 static_cast<uint16_t>(messageIndex + 3U),
                 0,
                 0}));
    }

    ::docan::DoCanNormalAddressingFilter<DataLinkLayerType> _doCanAddressingFilter{
        doCanMappingEntries, ::estd::make_slice(codecEntries)};
    ::docan::declare::DoCanTransportLayerConfig<DataLinkLayerType, NoOfMessages, NoOfMessages, 64U>
        _doCanConfig(_doCanParameters);
    ::estd::declare::vector<DoCanIsoLayer, NoOfMessages> _doCanIsoLayers;
    ::docan::DoCanTransportLayerContainer<DataLinkLayerType> _doCanIsoLayerContainer(
        _doCanIsoLayers);
    ::estd::declare::vector<::docan::DoCanPhysicalCanTransceiver<AddressingCodec>, NoOfMessages>
        _doCanPhysicalTransceivers;

    ::estd::declare::vector<CanTransceiver, NoOfMessages> canTransceivers;

    TickGeneratorAdapter _doCanTickGenerator;
    ::can::ICANFrameSentListener* canFrameSentListener[NoOfMessages];
    ::docan::IDoCanFrameReceiver<DataLinkLayerType>* canFrameReceiver[NoOfMessages];
    for (uint16_t messageIndex = 0; messageIndex < NoOfMessages; ++messageIndex)
    {
        ::common::busid::internal::Id<uint8_t, 0U> id;
        canTransceivers.emplace_back().construct(id);
        ::docan::DoCanPhysicalCanTransceiver<AddressingCodec>& doCanTransceiver
            = _doCanPhysicalTransceivers.emplace_back().construct(
                ::estd::by_ref(canTransceivers[messageIndex]),
                ::estd::by_ref(_doCanAddressingFilter),
                _doCanAddressingFilter,
                _doCanCodecClassic);
        canFrameSentListener[messageIndex] = &doCanTransceiver;
        _doCanIsoLayers.emplace_back().construct(
            id,
            _context,
            _doCanAddressingFilter,
            doCanTransceiver,
            _doCanTickGenerator,
            _doCanConfig,
            0U);
        canFrameReceiver[messageIndex] = &_doCanIsoLayers[messageIndex];
    }

    _doCanIsoLayerContainer.init();

    ::transport::BufferedTransportMessage<MessageSize> transportMessage[NoOfMessages];
    uint8_t data[NoOfMessages][MessageSize];
    bool sending[NoOfMessages];
    ::estd::declare::vector<TransportMessageProcessedListener, NoOfMessages>
        tpMessageProcessedListeners;
    for (size_t messageIndex = 0; messageIndex < NoOfMessages; ++messageIndex)
    {
        for (size_t byteIndex = 0; byteIndex < sizeof(data[messageIndex]); ++byteIndex)
        {
            data[messageIndex][byteIndex] = (messageIndex + byteIndex) % 0xFF;
        }
        transportMessage[messageIndex].setSourceId(messageIndex + 3);
        transportMessage[messageIndex].setTargetId(messageIndex + 2);
        transportMessage[messageIndex].append(data[messageIndex], sizeof(data[messageIndex]));
        transportMessage[messageIndex].setPayloadLength(sizeof(data[messageIndex]));

        sending[messageIndex] = false;
        tpMessageProcessedListeners.emplace_back().construct(nowUs, sending[messageIndex]);
    }

    _context.handleExecute();

    for (auto _ : state)
    {
        ++nowUs;
        _doCanIsoLayerContainer.cyclicTask(nowUs);
        for (size_t messageIndex = 0; messageIndex < NoOfMessages; ++messageIndex)
        {
            auto& doCanIsoTransportLayer = _doCanIsoLayers[messageIndex];
            sending[messageIndex]        = true;
            ASSERT_EQ(
                doCanIsoTransportLayer.send(
                    transportMessage[messageIndex], &tpMessageProcessedListeners[messageIndex]),
                ::transport::AbstractTransportLayer::ErrorCode::TP_OK);
            _context.execute();
            canFrameSentListener[messageIndex]->canFrameSent({});
            _context.execute();
            canFrameReceiver[messageIndex]->flowControlFrameReceived(
                messageIndex, ::docan::DoCanConstants::FlowStatus::CTS, 0U, 0U);
        }
        bool someSending = true;
        while (someSending)
        {
            someSending = false;
            for (size_t index = 0; index < NoOfMessages; ++index)
            {
                if (sending[index])
                {
                    someSending = sending[index];
                    canFrameSentListener[index]->canFrameSent({});
                    _context.execute();
                }
            }
        }
    }
}

BENCHMARK_TEMPLATE(TransmissionMultipleTransportLayersFullSegmentedMessages, 10, 10);
BENCHMARK_TEMPLATE(TransmissionMultipleTransportLayersFullSegmentedMessages, 100, 10);
BENCHMARK_TEMPLATE(TransmissionMultipleTransportLayersFullSegmentedMessages, 10, 100);
BENCHMARK_TEMPLATE(TransmissionMultipleTransportLayersFullSegmentedMessages, 100, 100);

template<size_t MessageSize, uint16_t NoOfMessages>
void TransmissionMultipleFullSegmentedMessages(benchmark::State& state)
{
    nowUs = 0;
    ::async::TestContext _context{1};

    AddressingCodec _doCanCodecClassic;
    ::docan::DoCanParameters _doCanParameters{
        ::estd::function<uint32_t()>::create<&nowUsFunc>(),
        ALLOCATE_TIMEOUT,
        RX_TIMEOUT,
        TX_CALLBACK_TIMEOUT,
        FLOW_CONTROL_TIMEOUT,
        ALLOCATE_RETRY_COUNT,
        FLOW_CONTROL_WAIT_COUNT,
        MIN_SEPARATION_TIME,
        BLOCK_SIZE};

    using FrameCodecType = ::docan::DoCanFrameCodec<DataLinkLayerType>;
    using MapperType     = ::docan::DoCanDefaultFrameSizeMapper<DataLinkLayerType::FrameSizeType>;

    MapperType const mapper;
    FrameCodecType const codecClassic(
        ::docan::DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    FrameCodecType const codecFd(::docan::DoCanFrameCodecConfigPresets::OPTIMIZED_FD, mapper);
    FrameCodecType const* codecEntries[2] = {&codecClassic, &codecFd};

    ::estd::declare::
        vector<::docan::DoCanNormalAddressingFilterAddressEntry<DataLinkLayerType>, NoOfMessages>
            doCanMappingEntries;
    for (uint16_t messageIndex = 0; messageIndex < NoOfMessages; ++messageIndex)
    {
        doCanMappingEntries.emplace_back().construct(
            ::docan::DoCanNormalAddressingFilterAddressEntry<DataLinkLayerType>(
                {messageIndex,
                 static_cast<uint16_t>(messageIndex + 1U),
                 static_cast<uint16_t>(messageIndex + 2U),
                 static_cast<uint16_t>(messageIndex + 3U),
                 0,
                 0}));
    }

    ::docan::DoCanNormalAddressingFilter<DataLinkLayerType> _doCanAddressingFilter{
        doCanMappingEntries, ::estd::make_slice(codecEntries)};
    ::docan::declare::DoCanTransportLayerConfig<DataLinkLayerType, NoOfMessages, NoOfMessages, 64U>
        _doCanConfig(_doCanParameters);
    ::estd::declare::vector<DoCanIsoLayer, 1> _doCanIsoLayers;
    ::docan::DoCanTransportLayerContainer<DataLinkLayerType> _doCanIsoLayerContainer(
        _doCanIsoLayers);

    ::common::busid::internal::Id<uint8_t, 0U> id;
    CanTransceiver canTransceivers(id);

    TickGeneratorAdapter _doCanTickGenerator;
    ::can::ICANFrameSentListener* canFrameSentListener;
    ::docan::IDoCanFrameReceiver<DataLinkLayerType>* canFrameReceiver;
    ::docan::DoCanPhysicalCanTransceiver<AddressingCodec> doCanTransceiver(
        canTransceivers, _doCanAddressingFilter, _doCanAddressingFilter, _doCanCodecClassic);
    canFrameSentListener = &doCanTransceiver;
    _doCanIsoLayers.emplace_back().construct(
        id,
        _context,
        _doCanAddressingFilter,
        doCanTransceiver,
        _doCanTickGenerator,
        _doCanConfig,
        0U);
    canFrameReceiver = &_doCanIsoLayers[0];

    _doCanIsoLayerContainer.init();

    ::transport::BufferedTransportMessage<MessageSize> transportMessage[NoOfMessages];
    uint8_t data[NoOfMessages][MessageSize];
    bool sending[NoOfMessages];
    ::estd::declare::vector<TransportMessageProcessedListener, NoOfMessages>
        tpMessageProcessedListeners;
    for (size_t messageIndex = 0; messageIndex < NoOfMessages; ++messageIndex)
    {
        for (size_t byteIndex = 0; byteIndex < sizeof(data[messageIndex]); ++byteIndex)
        {
            data[messageIndex][byteIndex] = (messageIndex + byteIndex) % 0xFF;
        }
        transportMessage[messageIndex].setSourceId(messageIndex + 3);
        transportMessage[messageIndex].setTargetId(messageIndex + 2);
        transportMessage[messageIndex].append(data[messageIndex], sizeof(data[messageIndex]));
        transportMessage[messageIndex].setPayloadLength(sizeof(data[messageIndex]));

        sending[messageIndex] = false;
        tpMessageProcessedListeners.emplace_back().construct(nowUs, sending[messageIndex]);
    }

    _context.handleExecute();

    for (auto _ : state)
    {
        nowUs++;
        for (size_t messageIndex = 0; messageIndex < NoOfMessages; ++messageIndex)
        {
            auto& doCanIsoTransportLayer = _doCanIsoLayers[0];
            sending[messageIndex]        = true;
            ASSERT_EQ(
                doCanIsoTransportLayer.send(
                    transportMessage[messageIndex], &tpMessageProcessedListeners[messageIndex]),
                ::transport::AbstractTransportLayer::ErrorCode::TP_OK);
            _context.execute();
            canFrameSentListener->canFrameSent({});
            _context.execute();
        }
        _doCanIsoLayerContainer.cyclicTask(nowUs);
        for (size_t messageIndex = 0; messageIndex < NoOfMessages; ++messageIndex)
        {
            canFrameReceiver->flowControlFrameReceived(
                messageIndex, ::docan::DoCanConstants::FlowStatus::CTS, 0U, 0U);
            _context.execute();
        }
        bool someSending = true;
        nowUs++;
        while (someSending)
        {
            someSending = false;
            for (size_t index = 0; index < NoOfMessages; ++index)
            {
                if (sending[index])
                {
                    someSending = sending[index];
                    canFrameSentListener->canFrameSent({});
                    _context.execute();
                }
            }
        }
        _doCanIsoLayerContainer.cyclicTask(nowUs);
        ASSERT_TRUE(_doCanConfig.getMessageTransmitterPool().full());
        _context.execute();
    }
}

BENCHMARK_TEMPLATE(TransmissionMultipleFullSegmentedMessages, 10, 10);
BENCHMARK_TEMPLATE(TransmissionMultipleFullSegmentedMessages, 100, 10);
BENCHMARK_TEMPLATE(TransmissionMultipleFullSegmentedMessages, 10, 100);
BENCHMARK_TEMPLATE(TransmissionMultipleFullSegmentedMessages, 100, 100);
