/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <async/Async.h>
#include <async/IRunnable.h>
#include <busid/BusId.h>
#include <can/DoCanMultiAddressingTransportLayer.h>
#include <docan/addressing/DoCanExtendedAddressingFilter.h>
#include <docan/addressing/DoCanNormalAddressing.h>
#include <docan/addressing/DoCanNormalAddressingFilter.h>
#include <docan/addressing/DoCanNormalFixedAddressing.h>
#include <docan/addressing/DoCanNormalFixedAddressingFilter.h>
#include <docan/addressing/DoCanRangeExtendedAddressingFilter.h>
#include <docan/can/DoCanPhysicalCanTransceiver.h>
#include <docan/datalink/DoCanDefaultFrameSizeMapper.h>
#include <docan/datalink/DoCanFdFrameSizeMapper.h>
#include <docan/datalink/DoCanFrameCodec.h>
#include <docan/transmitter/IDoCanTickGenerator.h>
#include <docan/transport/DoCanTransportLayerContainer.h>
#include <etl/optional.h>
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

/**
 * Showcases four ISO 15765-2 addressing schemes active at the same time on CAN_0, each able
 * to receive a physical and a functional UDS request:
 * - Normal Addressing (tester 0xF1; physical request 0x7E0/response 0x7E8, functional request
 *   0x7DF - the legislative OBD functional request identifier)
 * - Extended Addressing (tester 0xF4, explicit CAN identifiers 0x600/0x601)
 * - Range Extended Addressing (tester 0xF2, CAN identifiers 0x680-0x77F mapped onto the full 256
 *   transport addresses 0x00-0xFF; anchored at 0x680 rather than 0x700 so the range ends at
 *   0x77F, staying clear of the legislative Normal Addressing identifiers 0x7E0/0x7E8/0x7DF above)
 * - Normal Fixed Addressing (tester 0xF3, 29 bit CAN identifiers per ISO 15765-2)
 *
 * Since ITransportSystem only allows a single ::transport::AbstractTransportLayer to be
 * registered per bus id, the four inner ::docan::DoCanTransportLayer instances (one per
 * addressing scheme) are dispatched to via a single ::can::DoCanMultiAddressingTransportLayer
 * that is the only layer registered for CAN_0. See DoCanMultiAddressingTransportLayer for
 * details of the dispatch logic.
 */
class DoCanSystem final
: public ::lifecycle::AsyncLifecycleComponent
, private ::async::IRunnable
{
public:
    static size_t const NUM_CAN_TRANSPORT_LAYERS = 4UL;

    using NormalAddressingType = ::docan::DoCanNormalAddressing<>;
    using DataLinkLayerType    = NormalAddressingType::DataLinkLayerType;
    using ExtendedAddressingType
        = ::docan::DoCanExtendedAddressingFilter<DataLinkLayerType>::AddressingType;
    using RangeExtendedAddressingType
        = ::docan::DoCanRangeExtendedAddressingFilter<DataLinkLayerType>::AddressingType;
    using NormalFixedAddressingType = ::docan::DoCanNormalFixedAddressing<>;

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

    using FrameCodecType = ::docan::DoCanFrameCodec<DataLinkLayerType>;

    using NormalAddressingFilterType   = ::docan::DoCanNormalAddressingFilter<DataLinkLayerType>;
    using ExtendedAddressingFilterType = ::docan::DoCanExtendedAddressingFilter<DataLinkLayerType>;
    using RangeExtendedAddressingFilterType
        = ::docan::DoCanRangeExtendedAddressingFilter<DataLinkLayerType>;
    using NormalFixedAddressingFilterType
        = ::docan::DoCanNormalFixedAddressingFilter<DataLinkLayerType>;

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

    ::docan::DoCanFdFrameSizeMapper<DataLinkLayerType::FrameSizeType> _frameSizeMapper;
    FrameCodecType _classicCodec;
    FrameCodecType _extendedAddressingClassicCodec;

    NormalAddressingType _normalAddressing;
    ExtendedAddressingType _extendedAddressing;
    RangeExtendedAddressingType _rangeExtendedAddressing;
    NormalFixedAddressingType _normalFixedAddressing;

    NormalAddressingFilterType _normalAddressingFilter;
    ExtendedAddressingFilterType _extendedAddressingFilter;
    RangeExtendedAddressingFilterType _rangeExtendedAddressingFilter;
    NormalFixedAddressingFilterType _normalFixedAddressingFilter;

    ::docan::DoCanParameters _parameters;
    ::docan::declare::DoCanTransportLayerConfig<DataLinkLayerType, 80U, 15U, 64U>
        _transportLayerConfig;

    ::etl::optional<::docan::DoCanPhysicalCanTransceiver<NormalAddressingType>>
        _normalAddressingTransceiver;
    ::etl::optional<::docan::DoCanPhysicalCanTransceiver<ExtendedAddressingType>>
        _extendedAddressingTransceiver;
    ::etl::optional<::docan::DoCanPhysicalCanTransceiver<RangeExtendedAddressingType>>
        _rangeExtendedAddressingTransceiver;
    ::etl::optional<::docan::DoCanPhysicalCanTransceiver<NormalFixedAddressingType>>
        _normalFixedAddressingTransceiver;

    TransportLayers _transportLayers;
    TickGeneratorRunnableAdapter _tickGenerator;

    ::can::DoCanMultiAddressingTransportLayer _multiAddressingTransportLayer;

    FrameCodecType const* _codecs[1];
};

} // namespace docan
