// Copyright 2024 Accenture.

#include "systems/DoCanSystem.h"

#include "systems/ICanSystem.h"
#include "transport/ITransportSystem.h"

#include <app/appConfig.h>
#include <bsp/timer/SystemTimer.h>
#include <docan/common/DoCanLogger.h>
#include <docan/datalink/DoCanFrameCodecConfigPresets.h>

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

uint32_t systemUs() { return getSystemTimeUs32Bit(); }

} // namespace

namespace docan
{

DoCanSystem::AddressingFilterType::AddressEntryType DoCanSystem::_addresses[]
    = {{0x02A, 0x0F0U, 0x0F0U, LOGICAL_ADDRESS, 0, 0}};

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
      ::estd::function<decltype(systemUs)>::create<&systemUs>(),
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
        = _physicalTransceivers.emplace_back().construct(
            ::estd::by_ref(transceiver),
            ::estd::by_ref(_classicAddressingFilter),
            ::estd::by_ref(_classicAddressingFilter),
            ::estd::by_ref(_addressing));

    _transportLayers.createTransportLayer().construct(
        ::busid::CAN_0,
        ::estd::by_ref(_context),
        ::estd::by_ref(_classicAddressingFilter),
        ::estd::by_ref(doCanTransceiver),
        ::estd::by_ref(_tickGenerator),
        ::estd::by_ref(_transportLayerConfig),
        ::util::logger::DOCAN);
}

void DoCanSystem::init()
{
    _classicAddressingFilter.init(::estd::make_slice(_addresses), ::estd::make_slice(_codecs));

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
