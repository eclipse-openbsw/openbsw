// Copyright 2024 Accenture.

#pragma once

#include "transport/ITransportMessageProvidingListener.h"

#include <etl/span.h>

#include <gmock/gmock.h>

#include <set>

namespace transport
{
class TransportMessageProvidingListenerMock : public ITransportMessageProvidingListener
{
public:
    TransportMessageProvidingListenerMock(bool defaultImpl = true);

    virtual ~TransportMessageProvidingListenerMock() {}

    MOCK_METHOD(
        ReceiveResult,
        messageReceived,
        (uint8_t sourceBusId,
         TransportMessage& transportMessage,
         ITransportMessageProcessedListener* pNotificationListener));

    MOCK_METHOD(
        ErrorCode,
        getTransportMessage,
        (uint8_t srcBusId,
         uint16_t sourceId,
         uint16_t targetId,
         uint16_t size,
         ::etl::span<uint8_t const> const& peek,
         TransportMessage*& pTransportMessage));

    MOCK_METHOD(void, releaseTransportMessage, (TransportMessage & transportMessage));

    MOCK_METHOD(void, refuseSourceId, (uint8_t sourceId));

    MOCK_METHOD(void, refuseTargetId, (uint8_t targetId));

    MOCK_METHOD(void, dump, ());

    ErrorCode getTransportMessageImplementation(
        uint8_t srcBusId,
        uint16_t sourceId,
        uint16_t targetId,
        uint16_t size,
        ::etl::span<uint8_t const> const& peek,
        TransportMessage*& pTransportMessage);
    ReceiveResult messageReceivedImplementation(
        uint8_t sourceBusId,
        TransportMessage& transportMessage,
        ITransportMessageProcessedListener* pNotificationListener);
    void releaseTransportMessageImplementation(TransportMessage& transportMessage);

private:
    void refuseSourceIdImplementation(uint8_t sourceId);
    void refuseTargetIdImplementation(uint8_t targetId);

    std::set<uint8_t> fRefusedSourceIds;
    std::set<uint8_t> fRefusedTargetIds;
};

} // namespace transport
