/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "can/DoCanMultiAddressingTransportLayer.h"
#include "config/DoCanConfig.h"

#include <etl/error_handler.h>
#include <transport/TransportConfiguration.h>
#include <transport/TransportMessage.h>

namespace config = ::docan::config;

namespace can
{
DoCanMultiAddressingTransportLayer::DoCanMultiAddressingTransportLayer(uint8_t const busId)
: ::transport::AbstractTransportLayer(busId)
, _normalAddressingLayer(nullptr)
, _extendedAddressingLayer(nullptr)
, _rangeExtendedAddressingLayer(nullptr)
, _normalFixedAddressingLayer(nullptr)
, _normalFixedFunctionalAddressRemapper()
{}

void DoCanMultiAddressingTransportLayer::bind(
    TransportLayerType& normalAddressingLayer,
    TransportLayerType& extendedAddressingLayer,
    TransportLayerType& rangeExtendedAddressingLayer,
    TransportLayerType& normalFixedAddressingLayer)
{
    _normalAddressingLayer        = &normalAddressingLayer;
    _extendedAddressingLayer      = &extendedAddressingLayer;
    _rangeExtendedAddressingLayer = &rangeExtendedAddressingLayer;
    _normalFixedAddressingLayer   = &normalFixedAddressingLayer;
}

void DoCanMultiAddressingTransportLayer::propagateProvidingListener()
{
    TransportLayerType* const directLayers[]
        = {_normalAddressingLayer, _extendedAddressingLayer, _rangeExtendedAddressingLayer};
    for (TransportLayerType* const layer : directLayers)
    {
        ETL_ASSERT(layer != nullptr, ETL_ERROR_GENERIC("bind() must be called first"));
        layer->fProvidingListenerHelper.fpMessageProvider
            = fProvidingListenerHelper.fpMessageProvider;
        layer->fProvidingListenerHelper.fpMessageListener
            = fProvidingListenerHelper.fpMessageListener;
    }

    ETL_ASSERT(
        _normalFixedAddressingLayer != nullptr, ETL_ERROR_GENERIC("bind() must be called first"));
    _normalFixedFunctionalAddressRemapper.bind(
        *fProvidingListenerHelper.fpMessageProvider, *fProvidingListenerHelper.fpMessageListener);
    _normalFixedAddressingLayer->fProvidingListenerHelper.fpMessageProvider
        = &_normalFixedFunctionalAddressRemapper;
    _normalFixedAddressingLayer->fProvidingListenerHelper.fpMessageListener
        = &_normalFixedFunctionalAddressRemapper;
}

DoCanMultiAddressingTransportLayer::TransportLayerType*
DoCanMultiAddressingTransportLayer::getTransportLayerForTesterId(uint16_t const testerId) const
{
    switch (testerId)
    {
        case config::NORMAL_ADDRESSING_TESTER_ID:         return _normalAddressingLayer;
        case config::EXTENDED_ADDRESSING_TESTER_ID:       return _extendedAddressingLayer;
        case config::RANGE_EXTENDED_ADDRESSING_TESTER_ID: return _rangeExtendedAddressingLayer;
        case config::NORMAL_FIXED_ADDRESSING_TESTER_ID:   return _normalFixedAddressingLayer;
        default:                                          return nullptr;
    }
}

DoCanMultiAddressingTransportLayer::ErrorCode DoCanMultiAddressingTransportLayer::send(
    ::transport::TransportMessage& transportMessage,
    ::transport::ITransportMessageProcessedListener* const pNotificationListener)
{
    TransportLayerType* const layer = getTransportLayerForTesterId(transportMessage.getTargetId());

    if (layer == nullptr)
    {
        return ErrorCode::TP_SEND_FAIL;
    }
    return layer->send(transportMessage, pNotificationListener);
}

DoCanMultiAddressingTransportLayer::NormalFixedFunctionalAddressRemapper::
    NormalFixedFunctionalAddressRemapper()
: _provider(nullptr), _listener(nullptr)
{}

void DoCanMultiAddressingTransportLayer::NormalFixedFunctionalAddressRemapper::bind(
    ::transport::ITransportMessageProvider& provider,
    ::transport::ITransportMessageListener& listener)
{
    _provider = &provider;
    _listener = &listener;
}

DoCanMultiAddressingTransportLayer::NormalFixedFunctionalAddressRemapper::ErrorCode
DoCanMultiAddressingTransportLayer::NormalFixedFunctionalAddressRemapper::getTransportMessage(
    uint8_t const srcBusId,
    uint16_t const sourceAddress,
    uint16_t const targetAddress,
    uint16_t const size,
    ::etl::span<uint8_t const> const& peek,
    ::transport::TransportMessage*& pTransportMessage)
{
    return _provider->getTransportMessage(
        srcBusId, sourceAddress, remapTargetAddress(targetAddress), size, peek, pTransportMessage);
}

void DoCanMultiAddressingTransportLayer::NormalFixedFunctionalAddressRemapper::
    releaseTransportMessage(::transport::TransportMessage& transportMessage)
{
    _provider->releaseTransportMessage(transportMessage);
}

void DoCanMultiAddressingTransportLayer::NormalFixedFunctionalAddressRemapper::dump()
{
    _provider->dump();
}

DoCanMultiAddressingTransportLayer::NormalFixedFunctionalAddressRemapper::ReceiveResult
DoCanMultiAddressingTransportLayer::NormalFixedFunctionalAddressRemapper::messageReceived(
    uint8_t const sourceBusId,
    ::transport::TransportMessage& transportMessage,
    ::transport::ITransportMessageProcessedListener* const pNotificationListener)
{
    uint16_t const remappedTargetAddress = remapTargetAddress(transportMessage.getTargetId());
    if (remappedTargetAddress != transportMessage.getTargetId())
    {
        transportMessage.setTargetAddress(remappedTargetAddress);
    }
    return _listener->messageReceived(sourceBusId, transportMessage, pNotificationListener);
}

uint16_t
DoCanMultiAddressingTransportLayer::NormalFixedFunctionalAddressRemapper::remapTargetAddress(
    uint16_t const targetAddress)
{
    return (targetAddress == config::NORMAL_FIXED_FUNCTIONAL_ADDRESS)
               ? ::transport::TransportConfiguration::FUNCTIONAL_ALL_ISO14229
               : targetAddress;
}

} // namespace can
