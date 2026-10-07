/********************************************************************************
 * Copyright (c) 2026 Jefferson Nascimento
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

// AI disclosure: this file was largely generated with an AI assistant and was reviewed and
// tested by the contributor. Assisted-by: Anthropic Claude Opus 5.5

#pragma once

#include "transport/routing/DiagnosticRoute.h"
#include "transport/routing/IRouteObserver.h"
#include "transport/routing/TransportRouterStatistics.h"

#include <transport/AbstractTransportLayer.h>
#include <transport/ITransportMessageProcessedListener.h>
#include <transport/ITransportMessageProvidingListener.h>
#include <transport/TransportMessage.h>

#include <etl/array.h>
#include <etl/delegate.h>
#include <etl/intrusive_list.h>
#include <etl/uncopyable.h>

#include <cstdint>

namespace transport
{
/**
 * Diagnostic gateway router.
 *
 * Routes diagnostic messages by logical address between external testers, the gateway's own
 * diagnostic server and the nodes listed in a routing table:
 *
 * - A request from a tester (source in the tester range) to the local address goes to the
 *   local bus; to a route's address, to the route's bus with the gateway tester address as
 *   source; to the functional address, to the local bus and once to every route bus.
 * - A response from a route's node goes back to the tester that sent the pending request (or
 *   the functional request), on that tester's bus, with the original tester address.
 * - At most one request per route is outstanding. The router supervises P2 and P2*, passes
 *   response pending (NRC 0x78) through, and discards late or unsolicited responses.
 *
 * Errors are reported through ITransportMessageProvider::ErrorCode so that DoIP maps them to
 * diagnostic message NACKs: unknown target (0x03), too large (0x04), busy or no buffer (0x05).
 *
 * The router never interprets or generates diagnostic payloads for a node. Call cyclic()
 * periodically (e.g. every 10 ms) for timeout supervision.
 */
class TransportRouter
: public ITransportMessageProvidingListener
, public ::etl::uncopyable
{
public:
    using NowMsType = ::etl::delegate<uint32_t()>;

    enum class RouteState : uint8_t
    {
        IDLE,
        RESERVED,     ///< buffer handed to the tester side, request not yet forwarded
        SENDING,      ///< forwarded to the route's transport layer, delivery not confirmed
        WAIT_RESPONSE ///< waiting for the node within P2 or P2*
    };

    enum class ValidationError : uint8_t
    {
        NONE,
        NO_ROUTES,
        TOO_MANY_ROUTES,
        INVALID_TESTER_RANGE,
        ADDRESS_CONFLICT,
        DUPLICATE_ADDRESS,
        INVALID_TIMING,
        INVALID_LENGTH
    };

    static constexpr size_t NUM_BUFFERS                   = 4U;
    static constexpr uint16_t BUFFER_SIZE                 = 4095U;
    static constexpr size_t NUM_SMALL_BUFFERS             = 8U;
    static constexpr uint16_t SMALL_BUFFER_SIZE           = 8U;
    static constexpr size_t MAX_TESTERS                   = 8U;
    static constexpr size_t MAX_ROUTES                    = TransportRouterStatistics::MAX_ROUTES;
    static constexpr size_t INVALID_ROUTE                 = 0xFFU;
    static constexpr uint8_t INVALID_BUS                  = 0xFFU;
    /// Recommended TransportRouterConfiguration::transferTimeoutMs for unpaced classic CAN.
    static constexpr uint32_t DEFAULT_TRANSFER_TIMEOUT_MS = 2000U;

    TransportRouter(
        TransportRouterConfiguration const& configuration,
        TransportRouterStatistics& statistics,
        NowMsType nowMs);

    /// Checks the configuration; returns the first error and the index of the offending route.
    ValidationError validate(size_t& badRouteIndex) const;
    static char const* toString(ValidationError error);

    void init();
    void shutdown();
    void addTransportLayer(AbstractTransportLayer& transportLayer);
    void removeTransportLayer(AbstractTransportLayer& transportLayer);

    void setObserver(IRouteObserver* observer) { _observer = observer; }

    /// Timeout supervision.
    void cyclic();

    /// A tester disconnected: release its pending requests at once (late responses are then
    /// discarded).
    void releaseTester(uint16_t testerAddress);

    size_t routeIndex(uint16_t logicalAddress) const;
    RouteState routeState(size_t routeIndex) const;
    size_t freeBuffers() const;

    TransportRouterConfiguration const& configuration() const { return _configuration; }

    ErrorCode getTransportMessage(
        uint8_t srcBusId,
        uint16_t sourceAddress,
        uint16_t targetAddress,
        uint16_t size,
        ::etl::span<uint8_t const> const& peek,
        TransportMessage*& pTransportMessage) override;
    void releaseTransportMessage(TransportMessage& transportMessage) override;
    ReceiveResult messageReceived(
        uint8_t sourceBusId,
        TransportMessage& transportMessage,
        ITransportMessageProcessedListener* pNotificationListener) override;

    void dump() override {}

private:
    static constexpr uint8_t NO_ROUTE = 0xFFU;

    struct Slot
    {
        TransportMessage message;
        uint8_t* buffer;
        uint16_t size;
        bool locked;
        uint8_t route;
    };

    struct Tester
    {
        uint16_t address;
        uint8_t busId;
    };

    class RouteContext : public ITransportMessageProcessedListener
    {
    public:
        void transportMessageProcessed(
            TransportMessage& transportMessage, ProcessingResult result) override;

        TransportRouter* router                       = nullptr;
        uint8_t index                                 = 0U;
        RouteState state                              = RouteState::IDLE;
        uint16_t tester                               = 0U;
        uint8_t testerBusId                           = INVALID_BUS;
        uint32_t deadline                             = 0U;
        ITransportMessageProcessedListener* processed = nullptr;
    };

    class FunctionalCopyListener : public ITransportMessageProcessedListener
    {
    public:
        void transportMessageProcessed(
            TransportMessage& transportMessage, ProcessingResult result) override;

        TransportRouter* router = nullptr;
    };

    using TransportLayerList
        = ::etl::intrusive_list<AbstractTransportLayer, ::etl::bidirectional_link<0>>;

    bool isTester(uint16_t address) const;
    size_t routeFromNode(uint8_t busId, uint16_t sourceAddress) const;
    void learnTester(uint16_t address, uint8_t busId);
    uint8_t testerBus(uint16_t address) const;

    ErrorCode allocate(uint16_t size, uint8_t route, TransportMessage*& pTransportMessage);
    ReceiveResult
    forward(uint8_t busId, TransportMessage& message, ITransportMessageProcessedListener* listener);
    ReceiveResult routeRequest(
        size_t routeIndex,
        uint8_t testerBusId,
        TransportMessage& message,
        ITransportMessageProcessedListener* listener);
    ReceiveResult routeFunctional(
        uint8_t testerBusId,
        TransportMessage& message,
        ITransportMessageProcessedListener* listener);
    ReceiveResult routeNodeResponse(
        size_t routeIndex, TransportMessage& message, ITransportMessageProcessedListener* listener);
    void routeProcessed(RouteContext& route, bool success);
    void notify(size_t routeIndex, bool responded);

    static bool isResponsePending(TransportMessage const& message);

    static bool expired(uint32_t const now, uint32_t const deadline)
    {
        return static_cast<int32_t>(now - deadline) >= 0;
    }

    TransportRouterConfiguration _configuration;
    TransportRouterStatistics& _statistics;
    NowMsType _nowMs;
    IRouteObserver* _observer;
    TransportLayerList _transportLayers;

    ::etl::array<RouteContext, MAX_ROUTES> _routes;
    ::etl::array<Tester, MAX_TESTERS> _testers;
    size_t _nextTester;
    FunctionalCopyListener _functionalCopyListener;
    bool _functionalActive;
    uint16_t _functionalTester;
    uint8_t _functionalTesterBusId;
    uint32_t _functionalDeadline;

    uint8_t _buffers[NUM_BUFFERS][BUFFER_SIZE];
    uint8_t _smallBuffers[NUM_SMALL_BUFFERS][SMALL_BUFFER_SIZE];
    ::etl::array<Slot, NUM_BUFFERS + NUM_SMALL_BUFFERS> _slots;
};

} // namespace transport
