// Copyright 2024 Accenture.

#include "transport/AbstractTransportLayer.h"

#include "common/busid/BusId.h"
#include "transport/TransportLogger.h"

namespace transport
{
using ::util::logger::Logger;
using ::util::logger::TRANSPORT;

// explicit
AbstractTransportLayer::AbstractTransportLayer(uint8_t const busId)
: fBusId(busId), fProvidingListenerHelper(busId)
{}

// virtual
AbstractTransportLayer::ErrorCode AbstractTransportLayer::init() { return ErrorCode::TP_OK; }

// virtual
bool AbstractTransportLayer::shutdown(ShutdownDelegate) { return SYNC_SHUTDOWN_COMPLETE; }

/*
 *
 * TransportMessageProvidingListenerHelper
 *
 */

AbstractTransportLayer::TransportMessageProvidingListenerHelper::
    TransportMessageProvidingListenerHelper(uint8_t const busId)
: fBusId(busId), fpMessageProvider(nullptr), fpMessageListener(nullptr)
{}

// virtual
ITransportMessageProvidingListener::ErrorCode
AbstractTransportLayer::TransportMessageProvidingListenerHelper::getTransportMessage(
    uint8_t const srcBusId,
    uint16_t const sourceId,
    uint16_t const targetId,
    uint16_t const size,
    ::etl::span<uint8_t const> const& peek,
    TransportMessage*& pTransportMessage)
{
    if (fpMessageProvider != nullptr)
    {
        return fpMessageProvider->getTransportMessage(
            srcBusId, sourceId, targetId, size, peek, pTransportMessage);
    }
    Logger::warn(
        TRANSPORT,
        "AbstractTransportLayer(%s)::getTransportMessage() with no registered "
        "provider!",
        ::common::busid::BusIdTraits::getName(fBusId));
    pTransportMessage = nullptr;
    return ITransportMessageProvidingListener::ErrorCode::TPMSG_NO_MSG_AVAILABLE;
}

// virtual
void AbstractTransportLayer::TransportMessageProvidingListenerHelper::releaseTransportMessage(
    TransportMessage& transportMessage)
{
    if (fpMessageProvider != nullptr)
    {
        fpMessageProvider->releaseTransportMessage(transportMessage);
    }
}

// virtual
ITransportMessageListener::ReceiveResult
AbstractTransportLayer::TransportMessageProvidingListenerHelper::messageReceived(
    uint8_t const sourceBusId,
    TransportMessage& transportMessage,
    ITransportMessageProcessedListener* const pNotificationListener)
{
    if (fpMessageListener != nullptr)
    {
        return fpMessageListener->messageReceived(
            sourceBusId, transportMessage, pNotificationListener);
    }
    Logger::warn(
        TRANSPORT,
        "AbstractTransportLayer(%s)::messageReceived() with no registered "
        "listener!",
        ::common::busid::BusIdTraits::getName(fBusId));
    return ReceiveResult::RECEIVED_ERROR;
}

// virtual
void AbstractTransportLayer::TransportMessageProvidingListenerHelper::dump()
{
    if (fpMessageProvider != nullptr)
    {
        fpMessageProvider->dump();
    }
}

} // namespace transport
