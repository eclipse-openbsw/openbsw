#include "docan/addressing/DoCanNormalAddressing.h"
#include "docan/addressing/DoCanNormalAddressingFilter.h"
#include "docan/can/DoCanPhysicalCanTransceiver.h"
#include "docan/common/DoCanParameters.h"
#include "docan/datalink/DoCanDefaultFrameSizeMapper.h"
#include "docan/datalink/DoCanFrameCodecConfigPresets.h"
#include "docan/transmitter/IDoCanTickGenerator.h"
#include "docan/transport/DoCanTransportLayer.h"
#include "docan/transport/DoCanTransportLayerConfig.h"
#include "docan/transport/DoCanTransportLayerContainer.h"

#include <async/AsyncMock.h>
#include <can/canframes/CanId.h>
#include <can/transceiver/ICanTransceiverMock.h>
#include <etl/delegate.h>
#include <etl/span.h>
#include <transport/TransportMessage.h>
#include <transport/TransportMessageListenerMock.h>
#include <transport/TransportMessageProcessedListenerMock.h>
#include <transport/TransportMessageProviderMock.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// I rather despise the combination of singleton mocks to handle free functions and google benchmark
// (which keeps multiple instances of benchmarking fixtures alive simultaneously), as it forces
// these singletons to stand on their own and complicates their initialization logic
// AsyncMock must only be initialized once, so control initialization with a boolean. If this code
// is every multi threaded it'll have to become an atomic boolean
alignas(::testing::NiceMock<::async::AsyncMock>) uint8_t
    asyncMockMem[sizeof(::testing::NiceMock<::async::AsyncMock>)];
std::once_flag asyncMockInitialized;

uint32_t systemMicro() { return 0; }

TEST(DemoTest, DoCanIntegration)
{
    std::call_once(
        asyncMockInitialized,
        []() { new (asyncMockMem)::testing::NiceMock<::async::AsyncMock>(); });
    ::testing::Mock::AllowLeak(&asyncMockMem);

    // EXAMPLE_START DoCanNormalAddressing
    using Addressing    = ::docan::DoCanNormalAddressing<>;
    using DataLinkLayer = Addressing::DataLinkLayerType;
    Addressing addressing;
    // EXAMPLE_END DoCanNormalAddressing

    // EXAMPLE_START DoCanParameters
    uint16_t const ALLOCATE_TIMEOUT       = 1000U;
    uint16_t const RX_TIMEOUT             = 1000U;
    uint16_t const TX_CALLBACK_TIMEOUT    = 1000U;
    uint16_t const FLOW_CONTROL_TIMEOUT   = 1000U;
    uint8_t const ALLOCATE_RETRY_COUNT    = 1U;
    uint8_t const FLOW_CONTROL_WAIT_COUNT = 1U;
    uint16_t const MIN_SEPARATION_TIME    = 1U;
    uint8_t const BLOCK_SIZE              = 0U;

    ::docan::DoCanParameters parameters(
        ::etl::delegate<decltype(systemMicro)>::create<&systemMicro>(),
        ALLOCATE_TIMEOUT,
        RX_TIMEOUT,
        TX_CALLBACK_TIMEOUT,
        FLOW_CONTROL_TIMEOUT,
        ALLOCATE_RETRY_COUNT,
        FLOW_CONTROL_WAIT_COUNT,
        MIN_SEPARATION_TIME,
        BLOCK_SIZE);
    // EXAMPLE_END DoCanParameters

    // EXAMPLE_START DoCanNormalAddressingFilter
    constexpr uint16_t Ecu0Id = 0;
    constexpr uint16_t Ecu1Id = 10;
    constexpr uint16_t Ecu2Id = 20;
    constexpr uint16_t Ecu3Id = 30;

    // CAN reception IDs must be in ascending order
    constexpr ::docan::DoCanNormalAddressingFilterAddressEntry<DataLinkLayer> mappingEntriesCan1[]
        = {{// CAN reception ID - the CAN message ID used to receive data from the target ECU
            ::can::CanId::Base<101>::value,
            // CAN transmission ID - the CAN message ID used to transmit data to the target ECU
            ::can::CanId::Base<100>::value,
            // Transport source ID
            Ecu1Id,
            // Transport target ID
            Ecu0Id,
            // CAN frame reception codec index - relevant if there are multiple codecs available
            0,
            // CAN frame transmission codec index - relevant if there are multiple codecs available
            0},
           {::can::CanId::Base<201>::value, ::can::CanId::Base<200>::value, Ecu2Id, Ecu0Id, 0, 0},
           {::can::CanId::Base<301>::value, ::can::CanId::Base<300>::value, Ecu3Id, Ecu0Id, 0, 0}};

    using FrameCodecType = ::docan::DoCanFrameCodec<DataLinkLayer>;
    using MapperType     = ::docan::DoCanDefaultFrameSizeMapper<DataLinkLayer::FrameSizeType>;

    static MapperType const mapper;
    static FrameCodecType const codecClassic(
        ::docan::DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    static FrameCodecType const* codecEntries[] = {&codecClassic};

    ::docan::DoCanNormalAddressingFilter<DataLinkLayer> addressingFilter{
        ::etl::span<::docan::DoCanNormalAddressingFilterAddressEntry<DataLinkLayer> const>(
            mappingEntriesCan1),
        ::etl::span<FrameCodecType const*>(codecEntries)};
    // EXAMPLE_END DoCanNormalAddressingFilter

    uint8_t canBusId                  = 1;
    ::async::ContextType asyncContext = 1;
    uint8_t loggerComponent           = 0;

    ::testing::NiceMock<::can::ICanTransceiverMock> canTransceiver;
    ::testing::NiceMock<::transport::TransportMessageProviderMock> messageProvider;
    ::testing::NiceMock<::transport::TransportMessageListenerMock> messageListener;

    // EXAMPLE_START DoCanPhysicalCanTransceiver
    ::docan::DoCanPhysicalCanTransceiver<Addressing> transceiver(
        canTransceiver, addressingFilter, addressingFilter, addressing);

    // EXAMPLE_END DoCanPhysicalCanTransceiver

    // EXAMPLE_START IDoCanTickGenerator
    struct TickGenerator : public ::docan::IDoCanTickGenerator
    {
        // Schedule the stack to call the tick function frequently
        void tickNeeded() override {}
    };

    TickGenerator tickGenerator;
    // EXAMPLE_END IDoCanTickGenerator
    //
    // EXAMPLE_START DoCanTransportLayerConfig
    ::docan::declare::DoCanTransportLayerConfig<DataLinkLayer, 2U, 2U, 8U> transportLayerConfig(
        parameters);
    // EXAMPLE_END DoCanTransportLayerConfig

    // EXAMPLE_START DoCanTransportLayer
    ::docan::DoCanTransportLayer<DataLinkLayer> transportLayer(
        // ID of the CAN bus this transport layer will run on
        canBusId,
        // Async context in which this transport layer will schedule asynchronous calls
        asyncContext,
        // Filters incoming and outgoing requests to only receive/send valid, configured traffic
        addressingFilter,
        // transmitter/receiver which performs all CAN data transmission/reception
        transceiver,
        // generator of "ticks", used most often to speed up stack polling during segmented
        // transmissions
        tickGenerator,
        // configuration values for this transport layer
        transportLayerConfig,
        // logger component on which to write log messages
        loggerComponent);
    // EXAMPLE_END DoCanTransportLayer

    // EXAMPLE_START TransportConnection
    transportLayer.setTransportMessageProvider(&messageProvider);
    transportLayer.setTransportMessageListener(&messageListener);
    // EXAMPLE_END TransportConnection

    uint32_t nowUs        = 0;
    // TP layer shutdown callback takes no parameters and returns void
    auto shutdownLambda   = [](::transport::AbstractTransportLayer&) -> void {};
    auto shutdownCallback = ::transport::AbstractTransportLayer::ShutdownDelegate(shutdownLambda);

    // EXAMPLE_START Running
    // in initialization code
    transportLayer.init();

    // in normal running mode code - call every 10ms to guarantee 10ms response times
    transportLayer.cyclicTask(nowUs);

    // in shutdown code
    transportLayer.shutdown(shutdownCallback);
    // EXAMPLE_END Running

    ::transport::TransportMessage message;
    ::testing::NiceMock<::transport::TransportMessageProcessedListenerMock> notificationListener;

    // EXAMPLE_START SendingData
    transportLayer.send(message, &notificationListener);
    // EXAMPLE_END SendingData
}
