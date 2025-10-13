// Copyright 2024 Accenture.

#pragma once

#include <common/busid/BusId.h>
#include <etl/intrusive_list.h>
#include <etl/uncopyable.h>
#include <transport/AbstractTransportLayer.h>
#include <transport/ITransportMessageProvidingListener.h>
#include <transport/TransportConfiguration.h>
#include <transport/TransportMessage.h>

#include <platform/estdint.h>

namespace transport
{
class AbstractTransportLayer;

/**
 * Class for diagnostic routing.
 *
 *
 * \see ITransportMessageProvidingListener
 */
class TransportRouterSimple
: public ITransportMessageProvidingListener
, public etl::uncopyable
{
public:
    static uint8_t const NUM_BUFFERS             = 3U;
    static uint8_t const NUM_FUNCTIONAL_BUFFERS  = 8U;
    static uint16_t const BUFFER_SIZE            = 0xFFF;
    static uint16_t const FUNCTIONAL_BUFFER_SIZE = 8U;

    TransportRouterSimple();
    void init();
    void shutdown();

    ErrorCode getTransportMessage(
        uint8_t srcBusId,
        uint16_t sourceAddress,
        uint16_t targetId,
        uint16_t size,
        ::etl::span<uint8_t const> const& peek,
        TransportMessage*& pTransportMessage) override;

    void releaseTransportMessage(TransportMessage& transportMessage) override;

    ReceiveResult messageReceived(
        uint8_t sourceBusId,
        TransportMessage& transportMessage,
        ITransportMessageProcessedListener* pNotificationListener) override;

    void dump() override;

    void addTransportLayer(AbstractTransportLayer& transportLayer);
    void removeTransportLayer(AbstractTransportLayer& transportLayer);

private:
    void forwardMessageToTransportLayer(
        TransportMessage& transportMessage,
        uint8_t destBusId,
        ITransportMessageProcessedListener* pNotificationListener,
        AbstractTransportLayer::ErrorCode& result);

    typedef ::etl::intrusive_list<AbstractTransportLayer, etl::bidirectional_link<0>>
        TransportLayerList;

    bool _locked[NUM_BUFFERS];
    bool _functionalLocked[NUM_FUNCTIONAL_BUFFERS];
    uint8_t _buffer[NUM_BUFFERS][BUFFER_SIZE];
    uint8_t _functionalBuffer[NUM_FUNCTIONAL_BUFFERS][FUNCTIONAL_BUFFER_SIZE];
    TransportMessage _message[NUM_BUFFERS];
    TransportMessage _functionalMessage[NUM_FUNCTIONAL_BUFFERS];
    TransportLayerList _transportLayers;
    uint8_t _busIdToReply;
};

} // namespace transport
