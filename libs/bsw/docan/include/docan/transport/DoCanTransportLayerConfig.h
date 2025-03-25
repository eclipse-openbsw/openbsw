// Copyright 2024 Accenture.

#pragma once

#include "docan/common/DoCanParameters.h"
#include "docan/receiver/DoCanMessageReceiver.h"
#include "docan/transmitter/DoCanMessageTransmitter.h"

#include <util/estd/block_pool.h>

namespace docan
{
/**
 * Class holding parameters and object pools for CAN transport layer.
 * \tparam DataLinkLayer data link layer
 */
template<class DataLinkLayer>
class DoCanTransportLayerConfig
{
public:
    /**
     * Constructor.
     * \param messageReceiverPool reference to message receiver pool
     * \param messageTransmitterPool reference to message transmitter pool
     * \param parameters reference to parameters
     */
    DoCanTransportLayerConfig(
        ::util::estd::block_pool& messageReceiverPool,
        ::util::estd::block_pool& messageTransmitterPool,
        DoCanParameters const& parameters);

    /**
     * Get message receiver pool.
     * \return reference to receiver pool
     */
    ::util::estd::block_pool& getMessageReceiverPool() const;
    /**
     * Get message transmitter pool.
     * \return reference to transmitter pool
     */
    ::util::estd::block_pool& getMessageTransmitterPool() const;
    /**
     * Get access to parameters.
     * \return reference to parameters
     */
    DoCanParameters const& getParameters() const;

private:
    ::util::estd::block_pool& _messageReceiverPool;
    ::util::estd::block_pool& _messageTransmitterPool;
    DoCanParameters const& _parameters;
};

namespace declare
{

template<
    class DataLinkLayer,
    uint32_t RxCount,
    uint32_t TxCount,
    typename DataLinkLayer::FrameSizeType MaxFrameSize>
class DoCanTransportLayerConfig : public ::docan::DoCanTransportLayerConfig<DataLinkLayer>
{
public:
    explicit DoCanTransportLayerConfig(DoCanParameters const& parameters);

private:
    ::util::estd::declare::block_pool<
        RxCount,
        sizeof(::docan::declare::DoCanMessageReceiver<DataLinkLayer, MaxFrameSize>)>
        _messageReceiverPool;
    ::util::estd::declare::block_pool<TxCount, sizeof(DoCanMessageTransmitter<DataLinkLayer>)>
        _messageTransmitterPool;
};
} // namespace declare

/**
 * Inline implementation.
 */
template<class DataLinkLayer>
inline DoCanTransportLayerConfig<DataLinkLayer>::DoCanTransportLayerConfig(
    ::util::estd::block_pool& messageReceiverPool,
    ::util::estd::block_pool& messageTransmitterPool,
    DoCanParameters const& parameters)
: _messageReceiverPool(messageReceiverPool)
, _messageTransmitterPool(messageTransmitterPool)
, _parameters(parameters)
{}

template<class DataLinkLayer>
inline ::util::estd::block_pool&
DoCanTransportLayerConfig<DataLinkLayer>::getMessageReceiverPool() const
{
    return _messageReceiverPool;
}

template<class DataLinkLayer>
inline ::util::estd::block_pool&
DoCanTransportLayerConfig<DataLinkLayer>::getMessageTransmitterPool() const
{
    return _messageTransmitterPool;
}

template<class DataLinkLayer>
DoCanParameters const& DoCanTransportLayerConfig<DataLinkLayer>::getParameters() const
{
    return _parameters;
}

namespace declare
{

template<
    class DataLinkLayer,
    uint32_t RxCount,
    uint32_t TxCount,
    typename DataLinkLayer::FrameSizeType MaxFrameSize>
DoCanTransportLayerConfig<DataLinkLayer, RxCount, TxCount, MaxFrameSize>::DoCanTransportLayerConfig(
    DoCanParameters const& parameters)
: ::docan::DoCanTransportLayerConfig<DataLinkLayer>(
    _messageReceiverPool, _messageTransmitterPool, parameters)
, _messageReceiverPool()
, _messageTransmitterPool()
{}

} // namespace declare

} // namespace docan
