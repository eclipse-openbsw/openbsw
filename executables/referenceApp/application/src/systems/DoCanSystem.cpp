/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "systems/DoCanSystem.h"
#include "config/DoCanConfig.h"

#include "systems/ICanSystem.h"
#include "transport/ITransportSystem.h"

#include <app/appConfig.h>
#include <can/canframes/CanId.h>
#include <docan/common/DoCanLogger.h>
#include <docan/datalink/DoCanFrameCodecConfigPresets.h>
#include <etl/delegate.h>
#include <etl/span.h>
#include <time/TimestampProvider.h>
#include <transport/TransportConfiguration.h>

namespace
{
namespace config                      = ::docan::config;
uint32_t const TIMEOUT_DOCAN_SYSTEM   = 10U;
size_t const TICK_DELTA_TICKS         = 2U; // Tick delta
uint16_t const ALLOCATE_TIMEOUT       = 1000U;
uint16_t const RX_TIMEOUT             = 1000U;
uint16_t const TX_CALLBACK_TIMEOUT    = 1000U;
uint16_t const FLOW_CONTROL_TIMEOUT   = 1000U;
uint8_t const ALLOCATE_RETRY_COUNT    = 15U;
uint8_t const FLOW_CONTROL_WAIT_COUNT = 15U;
uint16_t const MIN_SEPARATION_TIME    = 200U;
uint8_t const BLOCK_SIZE              = 15U;

using DataLinkLayerType            = ::docan::DoCanSystem::DataLinkLayerType;
using NormalAddressingFilterType   = ::docan::DoCanNormalAddressingFilter<DataLinkLayerType>;
using ExtendedAddressingFilterType = ::docan::DoCanExtendedAddressingFilter<DataLinkLayerType>;
using RangeExtendedAddressingFilterType
    = ::docan::DoCanRangeExtendedAddressingFilter<DataLinkLayerType>;

// PLATFORM_SUPPORT_OBD_UDS_ADDRESSING and PLATFORM_SUPPORT_PROGRAMMING_SESSION
// are platform options (see the platforms' Options.cmake).
// PLATFORM_SUPPORT_OBD_UDS_ADDRESSING switches the DoCAN channel to the
// ISO 15765-4 OBD tester addressing (0x7E0 request / 0x7E8 response, logical
// address 0x0600 in appConfig.h) so off-the-shelf UDS tester tools talk to the
// board without a custom channel configuration. Platforms without the option
// keep the original example addressing. PLATFORM_SUPPORT_PROGRAMMING_SESSION
// adds an application-level UDS programming session (see
// udsConfiguration/src/uds/session/DiagSession.cpp) that keeps the UDS
// dispatcher alive instead of handing over to a bootloader.
NormalAddressingFilterType::AddressEntryType const NORMAL_ADDRESSING_ADDRESSES[]
#ifdef PLATFORM_SUPPORT_OBD_UDS_ADDRESSING
    = {{0x7E0U, 0x7E8U, 0x7E8U, LOGICAL_ADDRESS, 0, 0}};
#else
    = {{::can::CanId::Base<config::NORMAL_ADDRESSING_FUNCTIONAL_CAN_ID>::value,
        // ISO 15765-2 forbids multi-frame requests to a functional (broadcast) target, so this
        // entry deliberately reports an invalid transmission address rather than the real
        // response CAN id: DoCanReceiver rejects any multi-frame request whose transmission
        // address is invalid, while single-frame functional requests remain unaffected, since
        // the actual response is always addressed independently, using the real physical entry
        // below.
        DataLinkLayerType::INVALID_ADDRESS,
        config::NORMAL_ADDRESSING_TESTER_ID,
        ::transport::TransportConfiguration::FUNCTIONAL_ALL_ISO14229,
        0,
        0},
       {::can::CanId::Base<config::NORMAL_ADDRESSING_REQUEST_CAN_ID>::value,
        ::can::CanId::Base<config::NORMAL_ADDRESSING_RESPONSE_CAN_ID>::value,
        config::NORMAL_ADDRESSING_TESTER_ID,
        LOGICAL_ADDRESS,
        0,
        0}};
#endif

// mapping of each participant's raw CAN identifier to its own transport address, as needed by
// DoCanExtendedAddressingFilter. Entries must be ordered ascending by CAN identifier (duplicates
// allowed). The third entry deliberately reuses config::EXTENDED_ADDRESSING_RESPONSE_CAN_ID - the
// ECU's own, real response identifier - rather than introducing a separate (and therefore
// incorrect) one, so that even a (relatively unusual) multi-frame functional request would still
// cause any Flow Control frame to go out on the right CAN identifier, addressed back to the true
// sender.
ExtendedAddressingFilterType::AddressEntryType const EXTENDED_ADDRESSING_ADDRESSES[]
    = {{::can::CanId::Base<config::EXTENDED_ADDRESSING_REQUEST_CAN_ID>::value,
        config::EXTENDED_ADDRESSING_TESTER_ID},
       {::can::CanId::Base<config::EXTENDED_ADDRESSING_RESPONSE_CAN_ID>::value, LOGICAL_ADDRESS},
       {::can::CanId::Base<config::EXTENDED_ADDRESSING_RESPONSE_CAN_ID>::value,
        ::transport::TransportConfiguration::FUNCTIONAL_ALL_ISO14229}};

// functional (broadcast) addresses valid for extended addressing. Stored as a file-local static
// (rather than a local variable) since DoCanExtendedAddressingFilter::init() only stores a
// non-owning span over this data - a local/stack variable would dangle after init() returns.
uint16_t const EXTENDED_ADDRESSING_FUNCTIONAL_ADDRESSES[]
    = {::transport::TransportConfiguration::FUNCTIONAL_ALL_ISO14229};

// functional (broadcast) addresses valid for range extended addressing. Stored as a file-local
// static (rather than a local variable) since DoCanRangeExtendedAddressingFilter::init() only
// stores a non-owning span over this data - a local/stack variable would dangle after init()
// returns.
uint16_t const RANGE_EXTENDED_ADDRESSING_FUNCTIONAL_ADDRESSES[]
    = {::transport::TransportConfiguration::FUNCTIONAL_ALL_ISO14229};

// functional (group) addresses valid for normal fixed addressing. Stored as a file-local static
// (rather than a local variable) since DoCanNormalFixedAddressingFilter::init() only stores a
// non-owning span over this data - a local/stack variable would dangle after init() returns.
uint8_t const NORMAL_FIXED_FUNCTIONAL_ADDRESS[] = {config::NORMAL_FIXED_FUNCTIONAL_ADDRESS};

// Allowed testers for Normal Fixed
uint8_t const NORMAL_FIXED_ALLOWED_TESTERS[] = {config::NORMAL_FIXED_ADDRESSING_TESTER_ID};

uint32_t systemUs() { return ::bsw::time::TimestampProvider::getTimestampUs32Bit(); }

} // namespace

namespace docan
{

DoCanSystem::DoCanSystem(
    ::transport::ITransportSystem& transportSystem,
    ::can::ICanSystem& canSystem,
    ::async::ContextType asyncContext)
: _context(asyncContext)
, _cyclicTimeout()
, _canSystem(canSystem)
, _transportSystem(transportSystem)
, _frameSizeMapper()
, _classicCodec(::docan::DoCanFrameCodecConfigPresets::PADDED_CLASSIC, _frameSizeMapper)
, _extendedAddressingClassicCodec(
      ::docan::DoCanFrameCodecConfigPresets::EA_PADDED_CLASSIC, _frameSizeMapper)
, _normalAddressing()
, _extendedAddressing()
, _rangeExtendedAddressing()
, _normalFixedAddressing()
, _normalAddressingFilter()
, _extendedAddressingFilter()
, _rangeExtendedAddressingFilter()
, _normalFixedAddressingFilter()
, _parameters(
      ::etl::delegate<decltype(systemUs)>::create<&systemUs>(),
      ALLOCATE_TIMEOUT,
      RX_TIMEOUT,
      TX_CALLBACK_TIMEOUT,
      FLOW_CONTROL_TIMEOUT,
      ALLOCATE_RETRY_COUNT,
      FLOW_CONTROL_WAIT_COUNT,
      MIN_SEPARATION_TIME,
      BLOCK_SIZE)
, _transportLayerConfig(_parameters)
, _normalAddressingTransceiver()
, _extendedAddressingTransceiver()
, _rangeExtendedAddressingTransceiver()
, _normalFixedAddressingTransceiver()
, _transportLayers()
, _tickGenerator(asyncContext, _transportLayers)
, _multiAddressingTransportLayer(::busid::CAN_0)
, _codecs{&_classicCodec}
{
    setTransitionContext(asyncContext);
}

/**
 * Creates transport layers using the source and destination addresses.
 */
void DoCanSystem::initLayer()
{
    auto& transceiver = *_canSystem.getCanTransceiver(::busid::CAN_0);

    auto& normalAddressingTransceiver = _normalAddressingTransceiver.emplace(
        ::etl::ref(transceiver),
        ::etl::ref(_normalAddressingFilter),
        ::etl::ref(_normalAddressingFilter),
        ::etl::ref(_normalAddressing));

    auto& extendedAddressingTransceiver = _extendedAddressingTransceiver.emplace(
        ::etl::ref(transceiver),
        ::etl::ref(_extendedAddressingFilter),
        ::etl::ref(_extendedAddressingFilter),
        ::etl::ref(_extendedAddressing));

    auto& rangeExtendedAddressingTransceiver = _rangeExtendedAddressingTransceiver.emplace(
        ::etl::ref(transceiver),
        ::etl::ref(_rangeExtendedAddressingFilter),
        ::etl::ref(_rangeExtendedAddressingFilter),
        ::etl::ref(_rangeExtendedAddressing));

    auto& normalFixedAddressingTransceiver = _normalFixedAddressingTransceiver.emplace(
        ::etl::ref(transceiver),
        ::etl::ref(_normalFixedAddressingFilter),
        ::etl::ref(_normalFixedAddressingFilter),
        ::etl::ref(_normalFixedAddressing));

    auto& normalAddressingLayer = _transportLayers.emplace_back(
        ::busid::CAN_0,
        ::etl::ref(_context),
        ::etl::ref(_normalAddressingFilter),
        ::etl::ref(normalAddressingTransceiver),
        ::etl::ref(_tickGenerator),
        ::etl::ref(_transportLayerConfig),
        ::util::logger::DOCAN);

    auto& extendedAddressingLayer = _transportLayers.emplace_back(
        ::busid::CAN_0,
        ::etl::ref(_context),
        ::etl::ref(_extendedAddressingFilter),
        ::etl::ref(extendedAddressingTransceiver),
        ::etl::ref(_tickGenerator),
        ::etl::ref(_transportLayerConfig),
        ::util::logger::DOCAN);

    auto& rangeExtendedAddressingLayer = _transportLayers.emplace_back(
        ::busid::CAN_0,
        ::etl::ref(_context),
        ::etl::ref(_rangeExtendedAddressingFilter),
        ::etl::ref(rangeExtendedAddressingTransceiver),
        ::etl::ref(_tickGenerator),
        ::etl::ref(_transportLayerConfig),
        ::util::logger::DOCAN);

    auto& normalFixedAddressingLayer = _transportLayers.emplace_back(
        ::busid::CAN_0,
        ::etl::ref(_context),
        ::etl::ref(_normalFixedAddressingFilter),
        ::etl::ref(normalFixedAddressingTransceiver),
        ::etl::ref(_tickGenerator),
        ::etl::ref(_transportLayerConfig),
        ::util::logger::DOCAN);

    _multiAddressingTransportLayer.bind(
        normalAddressingLayer,
        extendedAddressingLayer,
        rangeExtendedAddressingLayer,
        normalFixedAddressingLayer);
}

void DoCanSystem::init()
{
    _normalAddressingFilter.init(
        ::etl::make_span(NORMAL_ADDRESSING_ADDRESSES), ::etl::make_span(_codecs));

    _extendedAddressingFilter.init(
        ::etl::make_span(EXTENDED_ADDRESSING_ADDRESSES),
        ::etl::make_span(EXTENDED_ADDRESSING_FUNCTIONAL_ADDRESSES),
        _extendedAddressingClassicCodec);

    _rangeExtendedAddressingFilter.init(
        config::RANGE_EXTENDED_ADDRESSING_BASE_CAN_ID,
        config::RANGE_EXTENDED_ADDRESSING_BASE_TRANSPORT_ID,
        config::RANGE_EXTENDED_ADDRESSING_COUNT,
        ::etl::make_span(RANGE_EXTENDED_ADDRESSING_FUNCTIONAL_ADDRESSES),
        _extendedAddressingClassicCodec);

    _normalFixedAddressingFilter.init(
        ::etl::make_span(NORMAL_FIXED_FUNCTIONAL_ADDRESS),
        ::etl::make_span(NORMAL_FIXED_ALLOWED_TESTERS),
        _classicCodec);

    initLayer();

    transitionDone();
}

/**
 * Adds the multi-addressing transport layer as the single routing target into the interface
 * transport system for CAN_0, then propagates its provider/listener wiring to the individual
 * inner (per-addressing-scheme) transport layers.
 */
void DoCanSystem::run()
{
    _transportSystem.addTransportLayer(_multiAddressingTransportLayer);
    _multiAddressingTransportLayer.propagateProvidingListener();

    _transportLayers.init();

    ::async::scheduleAtFixedRate(
        _context, *this, _cyclicTimeout, TIMEOUT_DOCAN_SYSTEM, ::async::TimeUnit::MILLISECONDS);

    transitionDone();
}

/**
 * Removes the multi-addressing transport layer and stops running the docan stack
 */
void DoCanSystem::shutdown()
{
    _cyclicTimeout.cancel();

    _transportSystem.removeTransportLayer(_multiAddressingTransportLayer);

    transitionDone();
}

void DoCanSystem::execute() { _transportLayers.cyclicTask(systemUs()); }

void DoCanSystem::TickGeneratorRunnableAdapter::scheduleTick()
{
    ::async::schedule(
        _context, *this, _tickTimeout, TICK_DELTA_TICKS * 100U, ::async::TimeUnit::MICROSECONDS);
}

DoCanSystem::TickGeneratorRunnableAdapter::TickGeneratorRunnableAdapter(
    ::async::ContextType const context, TransportLayers& layers)
: _layers(layers), _context(context)
{}

void DoCanSystem::TickGeneratorRunnableAdapter::cancelTimeout() { _tickTimeout.cancel(); }

void DoCanSystem::TickGeneratorRunnableAdapter::tickNeeded() { scheduleTick(); }

void DoCanSystem::TickGeneratorRunnableAdapter::execute()
{
    if (_layers.tick(systemUs()))
    {
        scheduleTick();
    }
}

} // namespace docan
