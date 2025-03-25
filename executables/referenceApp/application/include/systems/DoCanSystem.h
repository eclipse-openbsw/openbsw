// Copyright 2024 Accenture.

#pragma once

#include <async/Async.h>
#include <async/IRunnable.h>
#include <busid/BusId.h>
#include <docan/addressing/DoCanNormalAddressing.h>
#include <docan/addressing/DoCanNormalAddressingFilter.h>
#include <docan/can/DoCanPhysicalCanTransceiverContainer.h>
#include <docan/datalink/DoCanDefaultFrameSizeMapper.h>
#include <docan/datalink/DoCanFdFrameSizeMapper.h>
#include <docan/datalink/DoCanFrameCodec.h>
#include <docan/transmitter/IDoCanTickGenerator.h>
#include <docan/transport/DoCanTransportLayerContainer.h>
#include <lifecycle/AsyncLifecycleComponent.h>

namespace can
{
class ICanSystem;
} // namespace can

namespace transport
{
class ITransportSystem;
} // namespace transport

namespace docan
{

class DoCanSystem final
: public ::lifecycle::AsyncLifecycleComponent
, private ::async::IRunnable
{
public:
    static size_t const NUM_CAN_TRANSPORT_LAYERS = 1UL;

    using AddressingType    = ::docan::DoCanNormalAddressing<>;
    using DataLinkLayerType = AddressingType::DataLinkLayerType;

    DoCanSystem(
        ::transport::ITransportSystem& transportSystem,
        ::can::ICanSystem& canSystem,
        ::async::ContextType asyncContext);
    DoCanSystem(DoCanSystem const&)            = delete;
    DoCanSystem& operator=(DoCanSystem const&) = delete;

    void init() final;
    void run() final;
    void shutdown() final;

private:
    using TransportLayers = ::docan::declare::
        DoCanTransportLayerContainer<DataLinkLayerType, NUM_CAN_TRANSPORT_LAYERS>;

    using DoCanTransportLayerContainerType
        = ::docan::DoCanTransportLayerContainer<DataLinkLayerType>;

    using DoCanTransportLayerIt
        = DoCanTransportLayerContainerType::TransportLayerSliceType::iterator;

    using FrameCodecType = ::docan::DoCanFrameCodec<DataLinkLayerType>;

    using AddressingFilterType = ::docan::DoCanNormalAddressingFilter<DataLinkLayerType>;

    class TickGeneratorRunnableAdapter final
    : public ::docan::IDoCanTickGenerator
    , private ::async::RunnableType
    {
    public:
        TickGeneratorRunnableAdapter(::async::ContextType context, TransportLayers& layers);

        void cancelTimeout();

    private:
        void execute() final;
        void tickNeeded() final;
        void scheduleTick();

        TransportLayers& _layers;
        ::async::TimeoutType _tickTimeout;
        ::async::ContextType _context;
    };

    void execute() final;

    void initLayer();

    ::async::ContextType const _context;
    ::async::TimeoutType _cyclicTimeout;

    ::can::ICanSystem& _canSystem;
    ::transport::ITransportSystem& _transportSystem;

    AddressingType _addressing;
    ::docan::DoCanFdFrameSizeMapper<DataLinkLayerType::FrameSizeType> _frameSizeMapper;
    FrameCodecType _classicCodec;
    AddressingFilterType _classicAddressingFilter;

    ::docan::DoCanParameters _parameters;
    ::docan::declare::DoCanTransportLayerConfig<DataLinkLayerType, 80U, 15U, 64U>
        _transportLayerConfig;
    ::estd::declare::vector<DoCanPhysicalCanTransceiver<AddressingType>, NUM_CAN_TRANSPORT_LAYERS>
        _physicalTransceivers;
    TransportLayers _transportLayers;
    TickGeneratorRunnableAdapter _tickGenerator;

    FrameCodecType const* _codecs[1];

    static AddressingFilterType::AddressEntryType _addresses[];
};

} // namespace docan
