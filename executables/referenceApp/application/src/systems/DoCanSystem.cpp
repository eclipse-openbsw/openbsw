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

// tester address distinguishing Normal Addressing's own connections.
uint16_t const NORMAL_ADDRESSING_TESTER_ID = 0x0F1U;

// legislative (ISO 15765-4) normal addressing CAN identifiers for the first ECU, plus the
// legislative OBD functional (broadcast) request identifier, which every OBD-compliant ECU must
// also receive on (and reply to physically, via NORMAL_ADDRESSING_RESPONSE_CAN_ID above).
uint32_t const NORMAL_ADDRESSING_REQUEST_CAN_ID    = 0x7E0U;
uint32_t const NORMAL_ADDRESSING_RESPONSE_CAN_ID   = 0x7E8U;
uint32_t const NORMAL_ADDRESSING_FUNCTIONAL_CAN_ID = 0x7DFU;

uint32_t systemUs() { return ::bsw::time::TimestampProvider::getTimestampUs32Bit(); }

} // namespace

namespace docan
{

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
DoCanSystem::AddressingFilterType::AddressEntryType DoCanSystem::_addresses[]
#ifdef PLATFORM_SUPPORT_OBD_UDS_ADDRESSING
    = {{0x7E0U, 0x7E8U, 0x7E8U, LOGICAL_ADDRESS, 0, 0}};
#else
    = {{::can::CanId::Base<NORMAL_ADDRESSING_FUNCTIONAL_CAN_ID>::value,
        // ISO 15765-2 forbids multi-frame requests to a functional (broadcast) target, so this
        // entry deliberately reports an invalid transmission address rather than the real
        // response CAN id: DoCanReceiver rejects any multi-frame request whose transmission
        // address is invalid, while single-frame functional requests remain unaffected, since
        // the actual response is always addressed independently, using the real physical entry
        // below.
        DataLinkLayerType::INVALID_ADDRESS,
        NORMAL_ADDRESSING_TESTER_ID,
        ::transport::TransportConfiguration::FUNCTIONAL_ALL_ISO14229,
        0,
        0},
       {::can::CanId::Base<NORMAL_ADDRESSING_REQUEST_CAN_ID>::value,
        ::can::CanId::Base<NORMAL_ADDRESSING_RESPONSE_CAN_ID>::value,
        NORMAL_ADDRESSING_TESTER_ID,
        LOGICAL_ADDRESS,
        0,
        0}};
#endif

DoCanSystem::DoCanSystem(
    ::transport::ITransportSystem& transportSystem,
    ::can::ICanSystem& canSystem,
    ::async::ContextType asyncContext)
: _context(asyncContext)
, _cyclicTimeout()
, _canSystem(canSystem)
, _transportSystem(transportSystem)
, _addressing()
, _frameSizeMapper()
, _classicCodec(::docan::DoCanFrameCodecConfigPresets::PADDED_CLASSIC, _frameSizeMapper)
, _classicAddressingFilter()
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
, _physicalTransceivers()
, _transportLayers()
, _tickGenerator(asyncContext, _transportLayers)
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

    ::docan::DoCanPhysicalCanTransceiver<AddressingType>& doCanTransceiver
        = _physicalTransceivers.emplace_back(
            ::etl::ref(transceiver),
            ::etl::ref(_classicAddressingFilter),
            ::etl::ref(_classicAddressingFilter),
            ::etl::ref(_addressing));

    _transportLayers.emplace_back(
        ::busid::CAN_0,
        ::etl::ref(_context),
        ::etl::ref(_classicAddressingFilter),
        ::etl::ref(doCanTransceiver),
        ::etl::ref(_tickGenerator),
        ::etl::ref(_transportLayerConfig),
        ::util::logger::DOCAN);
}

void DoCanSystem::init()
{
    _classicAddressingFilter.init(::etl::make_span(_addresses), ::etl::make_span(_codecs));

    initLayer();

    transitionDone();
}

/**
 * Adds transport layers as a routing target into interface transport system
 */
void DoCanSystem::run()
{
    for (auto& layer : _transportLayers.getTransportLayers())
    {
        _transportSystem.addTransportLayer(layer);
    }
    _transportLayers.init();

    ::async::scheduleAtFixedRate(
        _context, *this, _cyclicTimeout, TIMEOUT_DOCAN_SYSTEM, ::async::TimeUnit::MILLISECONDS);

    transitionDone();
}

/**
 * Removes the transport layers and stops running the docan stack
 */
void DoCanSystem::shutdown()
{
    _cyclicTimeout.cancel();

    for (auto& layer : _transportLayers.getTransportLayers())
    {
        _transportSystem.removeTransportLayer(layer);
    }

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
