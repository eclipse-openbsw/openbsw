// Copyright 2024 Accenture.

#ifndef GUARD_6CE7A56B_DD21_43D1_B8C7_B40893A5CDB3
#define GUARD_6CE7A56B_DD21_43D1_B8C7_B40893A5CDB3

#include "docan/transport/DoCanTransportLayer.h"

#include <async/Types.h>

#include <estd/functional.h>
#include <estd/vector.h>

#include <limits>

namespace docan
{
/**
 * Container class that holds one or more transport layers.
 * \tparam DataLinkLayer data link layer class for transport layers
 */
template<class DataLinkLayer>
class DoCanTransportLayerContainer
{
public:
    using DataLinkLayerType        = DataLinkLayer;
    using TransportLayerType       = DoCanTransportLayer<DataLinkLayerType>;
    using TransportLayerSliceType  = ::estd::slice<TransportLayerType>;
    using TransportLayerVectorType = ::estd::vector<TransportLayerType>;

    using ShutdownCallbackType = ::estd::function<void()>;

    /**
     * Constructor.
     * \param layers reference to vector holding layers
     */
    explicit DoCanTransportLayerContainer(TransportLayerVectorType& layers);

    /**
     * Alocate a transport layer
     * \return constructor for the transport layer
     */
    ::estd::constructor<TransportLayerType> createTransportLayer();

    /**
     * Get access to slice of containers.
     * \return slice of containers
     */
    TransportLayerSliceType getTransportLayers() const;

    /**
     * Initialize all transport layers.
     */
    void init();
    /**
     * Shutdown all transport layers.
     * \param shutdownCallback function that will be called when all transport layers have
     * successfully shut down.
     */
    void shutdown(ShutdownCallbackType shutdownCallback);

    /**
     * Call cyclic task on all transport layers.
     */
    void cyclicTask(uint32_t nowUs);

    /**
     * Call tick on all transport layers.
     * \return true if any of the transport layers returns true
     */
    bool tick(uint32_t nowUs);

private:
    using ShutdownDelegate = ::transport::AbstractTransportLayer::ShutdownDelegate;

    void shutdownDone(::transport::AbstractTransportLayer& layer);
    void singleShutdownDone();

    TransportLayerVectorType& _layers;
    ShutdownCallbackType _shutdownCallback;
    uint8_t _shutdownPendingCount;
};

namespace declare
{
/**
 * Class that allocates a transport layer container that can hold a number of transport layers.
 * \tparam DataLinkLayer data link layer type
 * \tparam N number of transport layers to allocate
 */
template<class DataLinkLayer, size_t N>
class DoCanTransportLayerContainer : public ::docan::DoCanTransportLayerContainer<DataLinkLayer>
{
public:
    using TransportLayerType =
        typename ::docan::DoCanTransportLayerContainer<DataLinkLayer>::TransportLayerType;

    DoCanTransportLayerContainer();

private:
    ::estd::declare::vector<TransportLayerType, N> _layers;
};

/**
 * Class that helps in building a transport layer container.
 * \tparam DataLinkLayer data link layer type
 */
template<class DataLinkLayer>
class DoCanTransportLayerContainerBuilder
{
public:
    using DataLinkLayerType = DataLinkLayer;

    /**
     * Constructor for a transport layer builder.
     * \param addressConverter reference to address converter to use
     * \param tickGenerator reference to tick generator to use
     * \param config reference to config
     * \param loggerComponent reference to logger component
     */
    DoCanTransportLayerContainerBuilder(
        ::docan::DoCanTransportLayerContainer<DataLinkLayerType>& container,
        ::docan::IDoCanAddressConverter<DataLinkLayerType>& addressConverter,
        ::docan::IDoCanTickGenerator& tickGenerator,
        ::docan::DoCanTransportLayerConfig<DataLinkLayerType>& config,
        ::async::ContextType context,
        uint8_t loggerComponent);

    /**
     * Create a transport layer within the container.
     * \param busId bus identifier
     * \param transceiver transceiver to use
     */
    DoCanTransportLayer<DataLinkLayer>& addTransportLayer(
        uint8_t busId, ::docan::IDoCanPhysicalTransceiver<DataLinkLayerType>& transceiver);

private:
    ::docan::DoCanTransportLayerContainer<DataLinkLayerType>& _container;
    ::docan::IDoCanAddressConverter<DataLinkLayerType>& _addressConverter;
    ::docan::IDoCanTickGenerator& _tickGenerator;
    ::docan::DoCanTransportLayerConfig<DataLinkLayerType>& _config;
    ::async::ContextType const _context;
    uint8_t const _loggerComponent;
};

} // namespace declare

/**
 * Inline implementation.
 */
template<class DataLinkLayer>
DoCanTransportLayerContainer<DataLinkLayer>::DoCanTransportLayerContainer(
    TransportLayerVectorType& layers)
: _layers(layers), _shutdownCallback(), _shutdownPendingCount(0U)
{}

template<class DataLinkLayer>
inline ::estd::constructor<typename DoCanTransportLayerContainer<DataLinkLayer>::TransportLayerType>
DoCanTransportLayerContainer<DataLinkLayer>::createTransportLayer()
{
    return _layers.emplace_back();
}

template<class DataLinkLayer>
inline typename DoCanTransportLayerContainer<DataLinkLayer>::TransportLayerSliceType
DoCanTransportLayerContainer<DataLinkLayer>::getTransportLayers() const
{
    return _layers;
}

template<class DataLinkLayer>
void DoCanTransportLayerContainer<DataLinkLayer>::init()
{
    for (TransportLayerType& layer : _layers)
    {
        (void)layer.init();
    }
}

template<class DataLinkLayer>
void DoCanTransportLayerContainer<DataLinkLayer>::shutdown(
    ShutdownCallbackType const shutdownCallback)
{
    _shutdownPendingCount = 1U;
    _shutdownCallback     = shutdownCallback;
    for (TransportLayerType& layer : _layers)
    {
        {
            // Even if the value changes between now and the increment, we want to ensure this
            // function is never about to be incremented when _shutdownPendingCount is ever the max
            // possible value.
            estd_assert(
                _shutdownPendingCount
                != std::numeric_limits<decltype(_shutdownPendingCount)>::max());
            ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
            ++_shutdownPendingCount;
        }
        (void)layer.shutdown(ShutdownDelegate::create<
                             DoCanTransportLayerContainer<DataLinkLayer>,
                             &DoCanTransportLayerContainer<DataLinkLayer>::shutdownDone>(*this));
    }
    singleShutdownDone();
}

template<class DataLinkLayer>
void DoCanTransportLayerContainer<DataLinkLayer>::cyclicTask(uint32_t const nowUs)
{
    for (TransportLayerType& layer : _layers)
    {
        layer.cyclicTask(nowUs);
    }
}

template<class DataLinkLayer>
bool DoCanTransportLayerContainer<DataLinkLayer>::tick(uint32_t const nowUs)
{
    bool result = false;
    for (TransportLayerType& layer : _layers)
    {
        if (layer.tick(nowUs))
        {
            result = true;
        }
    }
    return result;
}

template<class DataLinkLayer>
void DoCanTransportLayerContainer<DataLinkLayer>::shutdownDone(
    ::transport::AbstractTransportLayer& /*layer*/)
{
    singleShutdownDone();
}

template<class DataLinkLayer>
void DoCanTransportLayerContainer<DataLinkLayer>::singleShutdownDone()
{
    bool done;
    {
        // Even if the value changes between now and the decrement, we want to ensure this function
        // is never called when _shutdownPendingCount is ever 0.
        estd_assert(_shutdownPendingCount != 0);
        ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
        --_shutdownPendingCount;
        done = (_shutdownPendingCount == 0U);
    }
    if (done)
    {
        _shutdownCallback();
    }
}

namespace declare
{
template<class DataLinkLayer, size_t N>
DoCanTransportLayerContainer<DataLinkLayer, N>::DoCanTransportLayerContainer()
: ::docan::DoCanTransportLayerContainer<DataLinkLayer>(_layers), _layers()
{}

template<class DataLinkLayer>
DoCanTransportLayerContainerBuilder<DataLinkLayer>::DoCanTransportLayerContainerBuilder(
    ::docan::DoCanTransportLayerContainer<DataLinkLayerType>& container,
    ::docan::IDoCanAddressConverter<DataLinkLayerType>& addressConverter,
    ::docan::IDoCanTickGenerator& tickGenerator,
    ::docan::DoCanTransportLayerConfig<DataLinkLayerType>& config,
    ::async::ContextType const context,
    uint8_t const loggerComponent)
: _container(container)
, _addressConverter(addressConverter)
, _tickGenerator(tickGenerator)
, _config(config)
, _context(context)
, _loggerComponent(loggerComponent)
{}

template<class DataLinkLayer>
DoCanTransportLayer<DataLinkLayer>&

DoCanTransportLayerContainerBuilder<DataLinkLayer>::addTransportLayer(
    uint8_t const busId, ::docan::IDoCanPhysicalTransceiver<DataLinkLayerType>& transceiver)
{
    return _container.createTransportLayer().construct(
        busId,
        ::estd::by_ref(_context),
        ::estd::by_ref(_addressConverter),
        ::estd::by_ref(transceiver),
        ::estd::by_ref(_tickGenerator),
        ::estd::by_ref(_config),
        _loggerComponent);
}

} // namespace declare

} // namespace docan

#endif // GUARD_6CE7A56B_DD21_43D1_B8C7_B40893A5CDB3
