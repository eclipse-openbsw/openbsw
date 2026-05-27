/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg): Logger/StringWriter API is variadic by design.

#include "transport/routing/TransportRouterSimple.h"

#include "busid/BusId.h"
#include "transport/ITransportMessageProcessedListener.h"

#include <async/Async.h>
#include <transport/TpRouterLogger.h>

namespace transport
{
using ::util::logger::Logger;
using ::util::logger::TPROUTER;

namespace
{
bool is1ByteDiagAddressBus(uint8_t const busId)
{
    return (busId != ::busid::ETH_0) && (busId != ::busid::ETH_1);
}
} // namespace

TransportRouterSimple::TransportRouterSimple() : _transportLayers()
{
    for (uint8_t i = 0U; i < NUM_BUFFERS; i++)
    {
        _locked[i] = false;
        _message[i].init(_buffer[i], BUFFER_SIZE);
    }
    for (uint8_t i = 0U; i < NUM_FUNCTIONAL_BUFFERS; i++)
    {
        _functionalLocked[i] = false;
        _functionalMessage[i].init(_functionalBuffer[i], FUNCTIONAL_BUFFER_SIZE);
    }
    _busIdToReply = ::busid::SELFDIAG;
}

void TransportRouterSimple::init() { _transportLayers.clear(); }

void TransportRouterSimple::shutdown() { _transportLayers.clear(); }

ITransportMessageProvidingListener::ErrorCode TransportRouterSimple::getTransportMessage(
    uint8_t const srcBusId,
    uint16_t const sourceAddress,
    uint16_t const targetId,
    uint16_t const size,
    ::etl::span<uint8_t const> const& /* peek */,
    TransportMessage*& pTransportMessage)
{
    Logger::debug(
        TPROUTER,
        "TransportRouterSimple::getTransportMessage : sourceAddress 0x%x, targetId 0x%x",
        sourceAddress,
        targetId);
    pTransportMessage = nullptr;

    ::async::LockType const lockGuard;
    uint16_t const targetId2Byte
        = is1ByteDiagAddressBus(srcBusId)
              ? TransportConfiguration::convert1ByteAddressTo2Byte(targetId)
              : targetId;

    if (TransportConfiguration::isFunctionalAddress(targetId2Byte))
    {
        if (size <= TransportConfiguration::MAX_FUNCTIONAL_MESSAGE_PAYLOAD_SIZE)
        {
            for (uint8_t i = 0U; i < NUM_FUNCTIONAL_BUFFERS; i++)
            {
                if (!_functionalLocked[i])
                {
                    _functionalMessage[i].init(_functionalBuffer[i], FUNCTIONAL_BUFFER_SIZE);
                    pTransportMessage    = &_functionalMessage[i];
                    _functionalLocked[i] = true;
                    return ErrorCode::TPMSG_OK;
                }
            }
        }
    }

    if (size > BUFFER_SIZE)
    {
        return ITransportMessageProvider::ErrorCode::TPMSG_SIZE_TOO_LARGE;
    }

    for (uint8_t i = 0U; i < NUM_BUFFERS; i++)
    {
        if (!_locked[i])
        {
            _message[i].init(_buffer[i], BUFFER_SIZE);
            pTransportMessage = &_message[i];
            _locked[i]        = true;
            return ErrorCode::TPMSG_OK;
        }
    }

    return ErrorCode::TPMSG_NO_MSG_AVAILABLE;
}

void TransportRouterSimple::releaseTransportMessage(TransportMessage& transportMessage)
{
    ::async::LockType const lockGuard;
    for (uint8_t i = 0U; i < NUM_BUFFERS; i++)
    {
        if (&transportMessage == &_message[i])
        {
            _locked[i] = false;
            return;
        }
    }
    for (uint8_t i = 0U; i < NUM_FUNCTIONAL_BUFFERS; i++)
    {
        if (&transportMessage == &_functionalMessage[i])
        {
            _functionalLocked[i] = false;
            return;
        }
    }
}

ITransportMessageProvidingListener::ReceiveResult TransportRouterSimple::messageReceived(
    uint8_t const sourceBusId,
    TransportMessage& transportMessage,
    ITransportMessageProcessedListener* const pNotificationListener)
{
    AbstractTransportLayer::ErrorCode result(AbstractTransportLayer::ErrorCode::TP_OK);

    if (is1ByteDiagAddressBus(sourceBusId))
    {
        transportMessage.setSourceAddress(
            TransportConfiguration::convert1ByteAddressTo2Byte(transportMessage.getSourceId()));
        transportMessage.setTargetAddress(
            TransportConfiguration::convert1ByteAddressTo2Byte(transportMessage.getTargetId()));
    }

    if ((sourceBusId == ::busid::CAN_0)
#ifdef PLATFORM_SUPPORT_ETHERNET
        || (sourceBusId == ::busid::ETH_0)
#endif
    )
    {
        _busIdToReply = sourceBusId;
        forwardMessageToTransportLayer(
            transportMessage, ::busid::SELFDIAG, pNotificationListener, result);
    }
    else if (sourceBusId == ::busid::SELFDIAG && _busIdToReply != ::busid::SELFDIAG)
    {
        forwardMessageToTransportLayer(
            transportMessage, _busIdToReply, pNotificationListener, result);
    }
    else
    {
        result = AbstractTransportLayer::ErrorCode::TP_SEND_FAIL;
    }

    if (result != AbstractTransportLayer::ErrorCode::TP_OK)
    {
        return ReceiveResult::RECEIVED_ERROR;
    }
    return ReceiveResult::RECEIVED_NO_ERROR;
}

void TransportRouterSimple::dump() {}

void TransportRouterSimple::addTransportLayer(AbstractTransportLayer& transportLayer)
{
    TransportLayerList::iterator itr         = _transportLayers.begin();
    TransportLayerList::iterator const e_itr = _transportLayers.end();
    for (; itr != e_itr; ++itr)
    {
        if (itr->getBusId() == transportLayer.getBusId())
        {
            Logger::error(
                TPROUTER,
                "TpLayer for bus %s must not be registered multiple times",
                ::common::busid::BusIdTraits::getName(itr->getBusId()));
            return;
        }
    }
    transportLayer.fProvidingListenerHelper.fpMessageListener = this;
    transportLayer.fProvidingListenerHelper.fpMessageProvider = this;
    _transportLayers.push_back(transportLayer);
}

void TransportRouterSimple::removeTransportLayer(AbstractTransportLayer& transportLayer)
{
    if (!_transportLayers.contains_node(transportLayer))
    {
        return;
    }
    transportLayer.fProvidingListenerHelper.fpMessageListener = nullptr;
    transportLayer.fProvidingListenerHelper.fpMessageProvider = nullptr;
    _transportLayers.erase(transportLayer);
}

void TransportRouterSimple::forwardMessageToTransportLayer(
    TransportMessage& transportMessage,
    uint8_t const destBusId,
    ITransportMessageProcessedListener* const pNotificationListener,
    AbstractTransportLayer::ErrorCode& result)
{
    if (is1ByteDiagAddressBus(destBusId))
    {
        transportMessage.setSourceAddress(
            TransportConfiguration::convert2ByteAddressTo1Byte(transportMessage.getSourceId()));
        transportMessage.setTargetAddress(
            TransportConfiguration::convert2ByteAddressTo1Byte(transportMessage.getTargetId()));
    }

    for (TransportLayerList::iterator itr = _transportLayers.begin(); itr != _transportLayers.end();
         ++itr)
    {
        if (itr->getBusId() == destBusId)
        {
            result = itr->send(transportMessage, pNotificationListener);
            break;
        }
    }
}

} // namespace transport

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
