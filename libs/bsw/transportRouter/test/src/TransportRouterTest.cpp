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

#include "transport/routing/TransportRouter.h"

#include "transport/AbstractTransportLayerMock.h"
#include "transport/TransportMessageProcessedListenerMock.h"
#include "transport/routing/RouteObserverMock.h"

#include <async/LockMock.h>
#include <etl/array.h>
#include <gtest/gtest.h>

#include <vector>

namespace
{
using namespace ::testing;
using namespace ::transport;

using ErrorCode     = ITransportMessageProvider::ErrorCode;
using ReceiveResult = ITransportMessageListener::ReceiveResult;
using TpError       = AbstractTransportLayer::ErrorCode;
using Processing    = ITransportMessageProcessedListener::ProcessingResult;
using RouteCounter  = TransportRouterStatistics::RouteCounter;
using RouterCounter = TransportRouterStatistics::RouterCounter;
using RouteState    = TransportRouter::RouteState;

uint8_t const BUS_ETH   = 1U; // DoIP testers
uint8_t const BUS_CAN_A = 2U;
uint8_t const BUS_CAN_B = 3U;
uint8_t const BUS_LOCAL = 4U; // gateway diagnostic server

uint16_t const LOCAL      = 0x1010U;
uint16_t const FUNCTIONAL = 0xE400U;
uint16_t const GW_TESTER  = 0x0E10U;
uint16_t const TESTER     = 0x0E80U;
uint16_t const TESTER_2   = 0x0E81U;
uint16_t const NODE_A     = 0x1020U; // on CAN A
uint16_t const NODE_B     = 0x1030U; // on CAN A
uint16_t const NODE_C     = 0x1040U; // on CAN B

uint32_t fakeNowMs = 0U;

uint32_t nowMs() { return fakeNowMs; }

DiagnosticRoute const ROUTES[] = {
    {NODE_A, BUS_CAN_A, 150U, 5000U, 4095U, "nodeA"},
    {NODE_B, BUS_CAN_A, 150U, 5000U, 4095U, "nodeB"},
    {NODE_C, BUS_CAN_B, 100U, 3000U, 64U, nullptr},
};

TransportRouterConfiguration configuration(::etl::span<DiagnosticRoute const> const routes)
{
    return TransportRouterConfiguration{
        LOCAL,
        BUS_LOCAL,
        FUNCTIONAL,
        GW_TESTER,
        0x0E00U,
        0x0EFFU,
        150U,
        TransportRouter::DEFAULT_TRANSFER_TIMEOUT_MS,
        7U,
        routes};
}

class TransportRouterTest : public Test
{
protected:
    TransportRouterTest()
    : _router(configuration(ROUTES), _statistics, TransportRouter::NowMsType::create<&nowMs>())
    , _eth(BUS_ETH)
    , _canA(BUS_CAN_A)
    , _canB(BUS_CAN_B)
    , _local(BUS_LOCAL)
    {}

    void SetUp() override
    {
        fakeNowMs = 1000U;
        // every lock must be released again
        EXPECT_CALL(_lock, lock()).Times(AnyNumber()).WillRepeatedly(Invoke([this] { ++_locks; }));
        EXPECT_CALL(_lock, unlock())
            .Times(AnyNumber())
            .WillRepeatedly(Invoke([this] { --_locks; }));
        _router.init();
        _router.addTransportLayer(_eth);
        _router.addTransportLayer(_canA);
        _router.addTransportLayer(_canB);
        _router.addTransportLayer(_local);
        _router.setObserver(&_observer);
    }

    void TearDown() override
    {
        EXPECT_EQ(0, _locks);
        _router.shutdown();
    }

    /// Requests a buffer like a transport layer does and returns the router's answer; used
    /// for requests the router refuses.
    ErrorCode requestBuffer(uint8_t busId, uint16_t source, uint16_t target, uint16_t length)
    {
        TransportMessage* msg = nullptr;
        return _router.getTransportMessage(busId, source, target, length, {}, msg);
    }

    /// Requests a buffer like a transport layer does, fills it and returns it.
    TransportMessage*
    message(uint8_t busId, uint16_t source, uint16_t target, std::vector<uint8_t> const& payload)
    {
        TransportMessage* msg = nullptr;
        EXPECT_EQ(
            ErrorCode::TPMSG_OK,
            _router.getTransportMessage(
                busId, source, target, static_cast<uint16_t>(payload.size()), {}, msg));
        if (msg == nullptr) // the failed expectation above is reported; avoid a crash
        {
            return nullptr;
        }
        msg->setSourceAddress(source);
        msg->setTargetAddress(target);
        msg->setPayloadLength(static_cast<uint16_t>(payload.size()));
        (void)msg->append(payload.data(), static_cast<uint16_t>(payload.size()));
        return msg;
    }

    /// Sends a physical request from TESTER to a node and confirms delivery on CAN.
    TransportMessage* sendRequest(
        uint16_t node,
        AbstractTransportLayerMock& bus,
        std::vector<uint8_t> const& payload = {0x22, 0xF1, 0x95})
    {
        TransportMessage* request = message(BUS_ETH, TESTER, node, payload);
        EXPECT_CALL(bus, send(_, _))
            .WillOnce(Invoke(
                [this](TransportMessage& msg, ITransportMessageProcessedListener* listener)
                {
                    _pendingListener = listener;
                    EXPECT_EQ(GW_TESTER, msg.getSourceId());
                    return TpError::TP_OK;
                }));
        EXPECT_EQ(
            ReceiveResult::RECEIVED_NO_ERROR, _router.messageReceived(BUS_ETH, *request, &_tester));
        return request;
    }

    /// The transport layer confirms delivery; the tester side then releases its buffer.
    void
    confirmDelivery(TransportMessage& request, Processing result = Processing::PROCESSED_NO_ERROR)
    {
        EXPECT_CALL(_tester, transportMessageProcessed(Ref(request), result))
            .WillOnce(Invoke(
                [this](TransportMessage& msg, Processing)
                {
                    EXPECT_EQ(TESTER, msg.getSourceId()) << "tester address restored";
                    _router.releaseTransportMessage(msg);
                }));
        _pendingListener->transportMessageProcessed(request, result);
    }

    /// A node response arriving on its bus; returns the router's result and keeps the
    /// response's target address after routing in _responseTarget.
    ReceiveResult nodeResponse(uint8_t busId, uint16_t node, std::vector<uint8_t> const& payload)
    {
        TransportMessage* response = message(busId, node, GW_TESTER, payload);
        ReceiveResult const result = _router.messageReceived(busId, *response, &_nodeSide);
        _responseTarget            = response->getTargetId();
        _router.releaseTransportMessage(*response);
        return result;
    }

    void advance(uint32_t ms)
    {
        fakeNowMs += ms;
        _router.cyclic();
    }

    /// Takes `count` buffers for messages of `length` bytes and appends them to `held`.
    void holdBuffers(std::vector<TransportMessage*>& held, size_t count, uint16_t length)
    {
        for (size_t i = 0U; i < count; ++i)
        {
            TransportMessage* msg = nullptr;
            EXPECT_EQ(
                ErrorCode::TPMSG_OK,
                _router.getTransportMessage(BUS_LOCAL, LOCAL, TESTER, length, {}, msg));
            held.push_back(msg);
        }
    }

    void releaseAll(std::vector<TransportMessage*> const& held)
    {
        for (TransportMessage* msg : held)
        {
            _router.releaseTransportMessage(*msg);
        }
    }

    StrictMock<::async::LockMock> _lock;
    int _locks = 0;
    TransportRouterStatistics _statistics;
    TransportRouter _router;
    StrictMock<AbstractTransportLayerMock> _eth;
    StrictMock<AbstractTransportLayerMock> _canA;
    StrictMock<AbstractTransportLayerMock> _canB;
    StrictMock<AbstractTransportLayerMock> _local;
    StrictMock<TransportMessageProcessedListenerMock> _tester;
    StrictMock<TransportMessageProcessedListenerMock> _nodeSide;
    StrictMock<RouteObserverMock> _observer;
    ITransportMessageProcessedListener* _pendingListener = nullptr;
    uint16_t _responseTarget                             = 0U;
};

// --- configuration ---------------------------------------------------------------------------

/**
 * Test that a consistent routing table passes validate() without naming a route.
 */
TEST_F(TransportRouterTest, ValidConfigurationPassesValidation)
{
    size_t bad = 0U;
    EXPECT_EQ(TransportRouter::ValidationError::NONE, _router.validate(bad));
    EXPECT_EQ(TransportRouter::INVALID_ROUTE, bad);
    EXPECT_STREQ("ok", TransportRouter::toString(TransportRouter::ValidationError::NONE));
}

/**
 * Test that validate() rejects each kind of invalid route and reports its index.
 *
 * Covers address conflicts with the local, functional and tester addresses and the local bus,
 * a duplicate node address, invalid P2/P2* timing and invalid maximum lengths.
 */
TEST_F(TransportRouterTest, InvalidRouteIsRejectedWithItsIndex)
{
    struct Case
    {
        DiagnosticRoute route;
        TransportRouter::ValidationError error;
    };

    Case const cases[] = {
        {{LOCAL, BUS_CAN_A, 150U, 5000U, 100U, nullptr},
         TransportRouter::ValidationError::ADDRESS_CONFLICT},
        {{FUNCTIONAL, BUS_CAN_A, 150U, 5000U, 100U, nullptr},
         TransportRouter::ValidationError::ADDRESS_CONFLICT},
        {{0x0E55U, BUS_CAN_A, 150U, 5000U, 100U, nullptr},
         TransportRouter::ValidationError::ADDRESS_CONFLICT},
        {{GW_TESTER, BUS_CAN_A, 150U, 5000U, 100U, nullptr},
         TransportRouter::ValidationError::ADDRESS_CONFLICT},
        {{0x1050U, BUS_LOCAL, 150U, 5000U, 100U, nullptr},
         TransportRouter::ValidationError::ADDRESS_CONFLICT},
        {{NODE_A, BUS_CAN_B, 150U, 5000U, 100U, nullptr},
         TransportRouter::ValidationError::DUPLICATE_ADDRESS},
        {{0x1050U, BUS_CAN_A, 0U, 5000U, 100U, nullptr},
         TransportRouter::ValidationError::INVALID_TIMING},
        {{0x1050U, BUS_CAN_A, 300U, 200U, 100U, nullptr},
         TransportRouter::ValidationError::INVALID_TIMING},
        {{0x1050U, BUS_CAN_A, 150U, 5000U, 0U, nullptr},
         TransportRouter::ValidationError::INVALID_LENGTH},
        {{0x1050U, BUS_CAN_A, 150U, 5000U, 4096U, nullptr},
         TransportRouter::ValidationError::INVALID_LENGTH},
    };
    for (Case const& c : cases)
    {
        DiagnosticRoute const routes[] = {ROUTES[0], c.route};
        TransportRouterStatistics statistics;
        TransportRouter router(
            configuration(routes), statistics, TransportRouter::NowMsType::create<&nowMs>());
        size_t bad = 0U;
        EXPECT_EQ(c.error, router.validate(bad)) << c.route.logicalAddress;
        EXPECT_EQ(1U, bad);
        EXPECT_STRNE("unknown", TransportRouter::toString(c.error));
    }
}

/**
 * Test that validate() rejects an empty or oversized routing table, a tester range that
 * overlaps node addresses, a functional length above the small buffer and a zero transfer
 * budget.
 */
TEST_F(TransportRouterTest, InvalidGlobalConfigurationIsRejected)
{
    TransportRouterStatistics statistics;
    size_t bad = 0U;
    {
        TransportRouter router(
            configuration({}), statistics, TransportRouter::NowMsType::create<&nowMs>());
        EXPECT_EQ(TransportRouter::ValidationError::NO_ROUTES, router.validate(bad));
    }
    {
        DiagnosticRoute routes[TransportRouter::MAX_ROUTES + 1U];
        for (size_t i = 0U; i < (TransportRouter::MAX_ROUTES + 1U); ++i)
        {
            routes[i] = {static_cast<uint16_t>(0x2000U + i), BUS_CAN_A, 150U, 5000U, 100U, nullptr};
        }
        TransportRouter router(
            configuration(routes), statistics, TransportRouter::NowMsType::create<&nowMs>());
        EXPECT_EQ(TransportRouter::ValidationError::TOO_MANY_ROUTES, router.validate(bad));
    }
    {
        auto cfg             = configuration(ROUTES);
        cfg.testerAddressMin = 0x1000U;
        TransportRouter router(cfg, statistics, TransportRouter::NowMsType::create<&nowMs>());
        EXPECT_EQ(TransportRouter::ValidationError::INVALID_TESTER_RANGE, router.validate(bad));
    }
    {
        auto cfg                = configuration(ROUTES);
        cfg.maxFunctionalLength = TransportRouter::SMALL_BUFFER_SIZE + 1U;
        TransportRouter router(cfg, statistics, TransportRouter::NowMsType::create<&nowMs>());
        EXPECT_EQ(TransportRouter::ValidationError::INVALID_LENGTH, router.validate(bad));
    }
    {
        auto cfg              = configuration(ROUTES);
        cfg.transferTimeoutMs = 0U;
        TransportRouter router(cfg, statistics, TransportRouter::NowMsType::create<&nowMs>());
        EXPECT_EQ(TransportRouter::ValidationError::INVALID_TIMING, router.validate(bad));
    }
}

/**
 * Test that toString() names every validation error and returns "unknown" otherwise.
 */
TEST_F(TransportRouterTest, EveryValidationErrorHasAText)
{
    using E = TransportRouter::ValidationError;
    for (E const error :
         {E::NONE,
          E::NO_ROUTES,
          E::TOO_MANY_ROUTES,
          E::INVALID_TESTER_RANGE,
          E::ADDRESS_CONFLICT,
          E::DUPLICATE_ADDRESS,
          E::INVALID_TIMING,
          E::INVALID_LENGTH})
    {
        EXPECT_STRNE("unknown", TransportRouter::toString(error));
    }
    EXPECT_STREQ("unknown", TransportRouter::toString(static_cast<E>(0xFFU)));
}

/**
 * Test that validate() rejects local or functional addresses inside the tester range, equal
 * local and functional addresses, and a zero functional length.
 */
TEST_F(TransportRouterTest, InvalidAddressSetupIsRejected)
{
    TransportRouterStatistics statistics;
    size_t bad = 0U;
    auto check
        = [&](TransportRouterConfiguration const& cfg, TransportRouter::ValidationError expected)
    {
        TransportRouter router(cfg, statistics, TransportRouter::NowMsType::create<&nowMs>());
        EXPECT_EQ(expected, router.validate(bad));
    };
    auto cfg         = configuration(ROUTES);
    cfg.localAddress = 0x0E20U; // inside the tester range
    check(cfg, TransportRouter::ValidationError::INVALID_TESTER_RANGE);
    cfg                   = configuration(ROUTES);
    cfg.functionalAddress = 0x0E20U;
    check(cfg, TransportRouter::ValidationError::INVALID_TESTER_RANGE);
    cfg                   = configuration(ROUTES);
    cfg.functionalAddress = LOCAL;
    check(cfg, TransportRouter::ValidationError::INVALID_TESTER_RANGE);
    cfg                     = configuration(ROUTES);
    cfg.maxFunctionalLength = 0U;
    check(cfg, TransportRouter::ValidationError::INVALID_LENGTH);
}

/**
 * Test that a second transport layer for an already registered bus is ignored and that
 * removing an unregistered layer has no effect.
 */
TEST_F(TransportRouterTest, TransportLayerOfABusIsRegisteredOnlyOnce)
{
    StrictMock<AbstractTransportLayerMock> second(BUS_CAN_A);
    _router.addTransportLayer(second);
    EXPECT_EQ(nullptr, second.fProvidingListenerHelper.fpMessageListener);
    _router.removeTransportLayer(second); // not registered: no effect
    _router.removeTransportLayer(_canB);
    EXPECT_EQ(nullptr, _canB.fProvidingListenerHelper.fpMessageListener);
}

// --- local and unknown targets ---------------------------------------------------------------

/**
 * Test that a request to the gateway's own address gets a full-size buffer and is passed to
 * the local bus with the tester address unchanged.
 */
TEST_F(TransportRouterTest, RequestToLocalAddressGoesToLocalBus)
{
    TransportMessage* request = message(BUS_ETH, TESTER, LOCAL, {0x22, 0xF1, 0x90});
    ASSERT_NE(nullptr, request);
    EXPECT_EQ(TransportRouter::BUFFER_SIZE, request->getMaxPayloadLength());
    EXPECT_CALL(_local, send(Ref(*request), &_tester)).WillOnce(Return(TpError::TP_OK));
    EXPECT_EQ(
        ReceiveResult::RECEIVED_NO_ERROR, _router.messageReceived(BUS_ETH, *request, &_tester));
    EXPECT_EQ(TESTER, request->getSourceId()) << "local requests keep the tester address";
    EXPECT_EQ(1U, _statistics.get(RouterCounter::LOCAL_REQUESTS));
    _router.releaseTransportMessage(*request);
}

/**
 * Test that a response of the local diagnostic server goes to the bus the tester used.
 */
TEST_F(TransportRouterTest, LocalResponseGoesBackToTheTestersBus)
{
    TransportMessage* request = message(BUS_ETH, TESTER, LOCAL, {0x3E, 0x00});
    EXPECT_CALL(_local, send(_, _)).WillOnce(Return(TpError::TP_OK));
    _router.messageReceived(BUS_ETH, *request, &_tester);
    _router.releaseTransportMessage(*request);

    TransportMessage* response = message(BUS_LOCAL, LOCAL, TESTER, {0x7E, 0x00});
    ASSERT_NE(nullptr, response);
    EXPECT_CALL(_eth, send(Ref(*response), &_nodeSide)).WillOnce(Return(TpError::TP_OK));
    EXPECT_EQ(
        ReceiveResult::RECEIVED_NO_ERROR,
        _router.messageReceived(BUS_LOCAL, *response, &_nodeSide));
    _router.releaseTransportMessage(*response);
}

/**
 * Test that a local response to a tester that never sent a request is not forwarded.
 */
TEST_F(TransportRouterTest, LocalResponseToAnUnknownTesterIsAnError)
{
    TransportMessage* response = message(BUS_LOCAL, LOCAL, 0x0E99U, {0x7E, 0x00});
    EXPECT_EQ(
        ReceiveResult::RECEIVED_ERROR, _router.messageReceived(BUS_LOCAL, *response, &_nodeSide));
    _router.releaseTransportMessage(*response);
}

/**
 * Test that a request to an address without a route is rejected and counted.
 */
TEST_F(TransportRouterTest, UnknownTargetIsRejectedAndCounted)
{
    EXPECT_EQ(ErrorCode::TPMSG_INVALID_TGT_ADDRESS, requestBuffer(BUS_ETH, TESTER, 0x1099U, 2U));
    EXPECT_EQ(1U, _statistics.get(RouterCounter::UNKNOWN_TARGET));
}

/**
 * Test that messages from unknown addresses, or from a node address on the wrong bus, are not
 * accepted.
 */
TEST_F(TransportRouterTest, MessageFromAnUnknownSourceIsNotAccepted)
{
    TransportMessage* msg = nullptr;
    EXPECT_EQ(
        ErrorCode::TPMSG_NOT_RESPONSIBLE,
        _router.getTransportMessage(BUS_CAN_A, 0x1234U, GW_TESTER, 8U, {}, msg));
    // a node address on the wrong bus is not that node
    EXPECT_EQ(
        ErrorCode::TPMSG_NOT_RESPONSIBLE,
        _router.getTransportMessage(BUS_CAN_B, NODE_A, GW_TESTER, 8U, {}, msg));
    uint8_t buffer[8];
    TransportMessage stray(buffer, sizeof(buffer));
    stray.setSourceAddress(0x1234U);
    EXPECT_EQ(ReceiveResult::RECEIVED_ERROR, _router.messageReceived(BUS_CAN_A, stray, nullptr));
    stray.setSourceAddress(TESTER);
    stray.setTargetAddress(0x1099U);
    EXPECT_EQ(ReceiveResult::RECEIVED_ERROR, _router.messageReceived(BUS_ETH, stray, nullptr));
}

// --- physical routing ------------------------------------------------------------------------

/**
 * Test the physical routing of a request to a node and of its response back to the tester.
 *
 * The request leaves with the gateway's tester address; the route waits for the delivery and
 * then for the response, which goes to the original tester. Statistics and buffers are
 * updated.
 */
TEST_F(TransportRouterTest, PhysicalRequestAndResponseRoundTrip)
{
    TransportMessage* request = sendRequest(NODE_A, _canA);
    EXPECT_EQ(RouteState::SENDING, _router.routeState(0U));
    confirmDelivery(*request);
    EXPECT_EQ(RouteState::WAIT_RESPONSE, _router.routeState(0U));

    EXPECT_CALL(_eth, send(_, &_nodeSide))
        .WillOnce(Invoke(
            [](TransportMessage& msg, ITransportMessageProcessedListener*)
            {
                EXPECT_EQ(NODE_A, msg.getSourceId());
                EXPECT_EQ(0x62U, msg.getPayload()[0]);
                return TpError::TP_OK;
            }));
    EXPECT_CALL(_observer, routeResponded(0U));
    EXPECT_EQ(
        ReceiveResult::RECEIVED_NO_ERROR,
        nodeResponse(BUS_CAN_A, NODE_A, {0x62, 0xF1, 0x95, 0x01}));
    EXPECT_EQ(TESTER, _responseTarget);
    EXPECT_EQ(RouteState::IDLE, _router.routeState(0U));
    EXPECT_EQ(1U, _statistics.get(0U, RouteCounter::REQUESTS));
    EXPECT_EQ(1U, _statistics.get(0U, RouteCounter::RESPONSES));
    EXPECT_EQ(
        _router.freeBuffers(), TransportRouter::NUM_BUFFERS + TransportRouter::NUM_SMALL_BUFFERS);
}

/**
 * Test that a response that arrives before the delivery confirmation ends the request and that
 * the late confirmation does not reopen the route.
 */
TEST_F(TransportRouterTest, ResponseBeforeDeliveryConfirmationIsAccepted)
{
    TransportMessage* request = sendRequest(NODE_A, _canA);
    EXPECT_CALL(_eth, send(_, _)).WillOnce(Return(TpError::TP_OK));
    EXPECT_CALL(_observer, routeResponded(0U));
    EXPECT_EQ(ReceiveResult::RECEIVED_NO_ERROR, nodeResponse(BUS_CAN_A, NODE_A, {0x7E, 0x00}));
    confirmDelivery(*request); // late confirmation does not reopen the route
    EXPECT_EQ(RouteState::IDLE, _router.routeState(0U));
}

/**
 * Test that requests to nodes on different routes are outstanding at the same time.
 */
TEST_F(TransportRouterTest, RequestsToTwoRoutesAreOutstandingTogether)
{
    TransportMessage* a = sendRequest(NODE_A, _canA);
    TransportMessage* c = sendRequest(NODE_C, _canB);
    confirmDelivery(*a);
    confirmDelivery(*c);
    EXPECT_CALL(_eth, send(_, _)).Times(2).WillRepeatedly(Return(TpError::TP_OK));
    EXPECT_CALL(_observer, routeResponded(2U));
    EXPECT_CALL(_observer, routeResponded(0U));
    EXPECT_EQ(ReceiveResult::RECEIVED_NO_ERROR, nodeResponse(BUS_CAN_B, NODE_C, {0x7E, 0x00}));
    EXPECT_EQ(ReceiveResult::RECEIVED_NO_ERROR, nodeResponse(BUS_CAN_A, NODE_A, {0x7E, 0x00}));
}

/**
 * Test that a second request to a route with an outstanding request is rejected and counted.
 */
TEST_F(TransportRouterTest, SecondRequestToABusyRouteIsRejected)
{
    TransportMessage* request = sendRequest(NODE_A, _canA);
    EXPECT_EQ(ErrorCode::TPMSG_NO_MSG_AVAILABLE, requestBuffer(BUS_ETH, TESTER_2, NODE_A, 2U));
    EXPECT_EQ(1U, _statistics.get(0U, RouteCounter::NACK_BUSY));
    confirmDelivery(*request);
    EXPECT_CALL(_observer, routeTimedOut(0U));
    advance(200U);
}

/**
 * Test that a request above the route's maximum length is rejected and counted.
 */
TEST_F(TransportRouterTest, RequestLargerThanTheRouteLimitIsRejected)
{
    EXPECT_EQ(ErrorCode::TPMSG_SIZE_TOO_LARGE, requestBuffer(BUS_ETH, TESTER, NODE_C, 65U));
    EXPECT_EQ(1U, _statistics.get(2U, RouteCounter::NACK_TOO_LARGE));
    EXPECT_EQ(RouteState::IDLE, _router.routeState(2U));
}

/**
 * Test that releasing a request buffer that was never forwarded frees its route.
 */
TEST_F(TransportRouterTest, BufferReleasedBeforeForwardingFreesTheRoute)
{
    TransportMessage* request = message(BUS_ETH, TESTER, NODE_A, {0x22, 0xF1, 0x95});
    EXPECT_EQ(RouteState::RESERVED, _router.routeState(0U));
    _router.releaseTransportMessage(*request);
    EXPECT_EQ(RouteState::IDLE, _router.routeState(0U));
}

/**
 * Test that a request the node's transport layer refuses is an error, keeps the tester
 * address, is counted and frees the route when released.
 */
TEST_F(TransportRouterTest, FailedForwardingFreesTheRouteOnRelease)
{
    TransportMessage* request = message(BUS_ETH, TESTER, NODE_A, {0x22, 0xF1, 0x95});
    EXPECT_CALL(_canA, send(_, _)).WillOnce(Return(TpError::TP_SEND_FAIL));
    EXPECT_EQ(ReceiveResult::RECEIVED_ERROR, _router.messageReceived(BUS_ETH, *request, &_tester));
    EXPECT_EQ(TESTER, request->getSourceId());
    EXPECT_EQ(1U, _statistics.get(0U, RouteCounter::TX_FAILURES));
    _router.releaseTransportMessage(*request);
    EXPECT_EQ(RouteState::IDLE, _router.routeState(0U));
}

/**
 * Test that a request to a route whose bus has no transport layer is an error.
 */
TEST_F(TransportRouterTest, MissingTransportLayerIsAnError)
{
    _router.removeTransportLayer(_canB);
    TransportMessage* request = message(BUS_ETH, TESTER, NODE_C, {0x3E, 0x00});
    EXPECT_EQ(ReceiveResult::RECEIVED_ERROR, _router.messageReceived(BUS_ETH, *request, &_tester));
    _router.releaseTransportMessage(*request);
}

/**
 * Test that a failed delivery frees the route, is counted and is reported to the observer.
 */
TEST_F(TransportRouterTest, FailedDeliveryFreesTheRouteAndReportsTheNode)
{
    TransportMessage* request = sendRequest(NODE_A, _canA);
    EXPECT_CALL(_observer, routeTimedOut(0U));
    confirmDelivery(*request, Processing::PROCESSED_ERROR_TIMEOUT);
    EXPECT_EQ(RouteState::IDLE, _router.routeState(0U));
    EXPECT_EQ(1U, _statistics.get(0U, RouteCounter::TX_FAILURES));
}

// --- timing ----------------------------------------------------------------------------------

/**
 * Test that a missing response within P2 frees the route and is reported to the observer.
 */
TEST_F(TransportRouterTest, NoResponseWithinP2FreesTheRoute)
{
    TransportMessage* request = sendRequest(NODE_A, _canA);
    confirmDelivery(*request);
    advance(149U);
    EXPECT_EQ(RouteState::WAIT_RESPONSE, _router.routeState(0U));
    EXPECT_CALL(_observer, routeTimedOut(0U));
    advance(1U);
    EXPECT_EQ(RouteState::IDLE, _router.routeState(0U));
    EXPECT_EQ(1U, _statistics.get(0U, RouteCounter::TIMEOUTS));
}

/**
 * Test that a delivery that is never confirmed times out after the transfer budget.
 */
TEST_F(TransportRouterTest, UnconfirmedDeliveryTimesOut)
{
    sendRequest(NODE_A, _canA);
    EXPECT_CALL(_observer, routeTimedOut(0U));
    advance(TransportRouter::DEFAULT_TRANSFER_TIMEOUT_MS);
    EXPECT_EQ(RouteState::IDLE, _router.routeState(0U));
}

/**
 * Test that NRC 0x78 (response pending) is forwarded and extends the deadline to P2*.
 */
TEST_F(TransportRouterTest, ResponsePendingIsForwardedAndExtendsToP2Star)
{
    TransportMessage* request = sendRequest(NODE_A, _canA);
    confirmDelivery(*request);
    EXPECT_CALL(_eth, send(_, _)).Times(2).WillRepeatedly(Return(TpError::TP_OK));
    EXPECT_EQ(
        ReceiveResult::RECEIVED_NO_ERROR, nodeResponse(BUS_CAN_A, NODE_A, {0x7F, 0x22, 0x78}));
    advance(4999U);
    EXPECT_EQ(RouteState::WAIT_RESPONSE, _router.routeState(0U));
    EXPECT_CALL(_observer, routeResponded(0U));
    EXPECT_EQ(
        ReceiveResult::RECEIVED_NO_ERROR, nodeResponse(BUS_CAN_A, NODE_A, {0x62, 0xF1, 0x95}));
    EXPECT_EQ(1U, _statistics.get(0U, RouteCounter::PENDING));
    EXPECT_EQ(1U, _statistics.get(0U, RouteCounter::RESPONSES));
}

/**
 * Test that a request with a pending response times out when P2* expires.
 */
TEST_F(TransportRouterTest, ResponsePendingTimesOutAfterP2Star)
{
    TransportMessage* request = sendRequest(NODE_A, _canA);
    confirmDelivery(*request);
    EXPECT_CALL(_eth, send(_, _)).WillOnce(Return(TpError::TP_OK));
    nodeResponse(BUS_CAN_A, NODE_A, {0x7F, 0x22, 0x78});
    EXPECT_CALL(_observer, routeTimedOut(0U));
    advance(5000U);
    EXPECT_EQ(RouteState::IDLE, _router.routeState(0U));
}

/**
 * Test that the reception of a long response may outlast P2 once its first frame arrived.
 */
TEST_F(TransportRouterTest, SegmentedResponseGetsMoreThanP2)
{
    TransportMessage* request = sendRequest(NODE_A, _canA);
    confirmDelivery(*request);
    advance(100U);
    // first frame of a long response: the reception may outlast P2
    TransportMessage* response
        = message(BUS_CAN_A, NODE_A, GW_TESTER, std::vector<uint8_t>(1000U, 0x62U));
    advance(1000U);
    EXPECT_EQ(RouteState::WAIT_RESPONSE, _router.routeState(0U));
    EXPECT_CALL(_eth, send(_, _)).WillOnce(Return(TpError::TP_OK));
    EXPECT_CALL(_observer, routeResponded(0U));
    EXPECT_EQ(
        ReceiveResult::RECEIVED_NO_ERROR,
        _router.messageReceived(BUS_CAN_A, *response, &_nodeSide));
    _router.releaseTransportMessage(*response);
}

/**
 * Test that deadlines are correct when the millisecond timer wraps around.
 */
TEST_F(TransportRouterTest, DeadlinesWorkAcrossTimerWrapAround)
{
    fakeNowMs                 = 0xFFFFFFF0U;
    TransportMessage* request = sendRequest(NODE_A, _canA);
    confirmDelivery(*request);
    advance(100U); // wraps
    EXPECT_EQ(RouteState::WAIT_RESPONSE, _router.routeState(0U));
    EXPECT_CALL(_observer, routeTimedOut(0U));
    advance(50U);
}

/**
 * Test that a negative response other than NRC 0x78 is final and ends the request.
 */
TEST_F(TransportRouterTest, NegativeResponseOtherThanPendingEndsTheRequest)
{
    TransportMessage* request = sendRequest(NODE_A, _canA);
    confirmDelivery(*request);
    EXPECT_CALL(_eth, send(_, _)).WillOnce(Return(TpError::TP_OK));
    EXPECT_CALL(_observer, routeResponded(0U));
    EXPECT_EQ(
        ReceiveResult::RECEIVED_NO_ERROR, nodeResponse(BUS_CAN_A, NODE_A, {0x7F, 0x22, 0x31}));
    EXPECT_EQ(RouteState::IDLE, _router.routeState(0U));
    EXPECT_EQ(1U, _statistics.get(0U, RouteCounter::RESPONSES));
}

/**
 * Test that a long response gets the configured transfer budget even when P2* is shorter.
 *
 * Uses a router with one unnamed route (P2* 500 ms); the reception times out exactly at
 * DEFAULT_TRANSFER_TIMEOUT_MS.
 */
TEST_F(TransportRouterTest, SegmentedResponseGetsTheTransferBudget)
{
    DiagnosticRoute const routes[] = {{NODE_A, BUS_CAN_A, 50U, 500U, 4095U, nullptr}};
    TransportRouterStatistics statistics;
    TransportRouter router(
        configuration(routes), statistics, TransportRouter::NowMsType::create<&nowMs>());
    // a transport layer belongs to one router at a time
    _router.removeTransportLayer(_eth);
    _router.removeTransportLayer(_canA);
    router.init();
    router.addTransportLayer(_eth);
    router.addTransportLayer(_canA);
    router.setObserver(&_observer);
    TransportMessage* request = nullptr;
    ASSERT_EQ(
        ErrorCode::TPMSG_OK, router.getTransportMessage(BUS_ETH, TESTER, NODE_A, 3U, {}, request));
    request->setSourceAddress(TESTER);
    request->setTargetAddress(NODE_A);
    EXPECT_CALL(_canA, send(_, _)).WillOnce(Return(TpError::TP_OK));
    EXPECT_EQ(ReceiveResult::RECEIVED_NO_ERROR, router.messageReceived(BUS_ETH, *request, nullptr));
    TransportMessage* response = nullptr;
    ASSERT_EQ(
        ErrorCode::TPMSG_OK,
        router.getTransportMessage(BUS_CAN_A, NODE_A, GW_TESTER, 500U, {}, response));
    fakeNowMs += TransportRouter::DEFAULT_TRANSFER_TIMEOUT_MS - 1U;
    router.cyclic();
    EXPECT_EQ(RouteState::SENDING, router.routeState(0U));
    EXPECT_CALL(_observer, routeTimedOut(0U));
    fakeNowMs += 1U;
    router.cyclic(); // unnamed route: logged without a name
    EXPECT_EQ(RouteState::IDLE, router.routeState(0U));
    router.releaseTransportMessage(*response);
    router.releaseTransportMessage(*request);
    EXPECT_EQ(RouteState::IDLE, router.routeState(TransportRouter::MAX_ROUTES));
    router.removeTransportLayer(_eth);
    router.removeTransportLayer(_canA);
}

/**
 * Test that a request without a processed listener is routed and its route times out
 * normally.
 */
TEST_F(TransportRouterTest, RequestWithoutProcessedListenerIsRouted)
{
    TransportMessage* request                    = message(BUS_ETH, TESTER, NODE_A, {0x3E, 0x00});
    ITransportMessageProcessedListener* listener = nullptr;
    EXPECT_CALL(_canA, send(_, _)).WillOnce(DoAll(SaveArg<1>(&listener), Return(TpError::TP_OK)));
    EXPECT_EQ(
        ReceiveResult::RECEIVED_NO_ERROR, _router.messageReceived(BUS_ETH, *request, nullptr));
    listener->transportMessageProcessed(*request, Processing::PROCESSED_NO_ERROR);
    _router.releaseTransportMessage(*request);
    EXPECT_EQ(RouteState::WAIT_RESPONSE, _router.routeState(0U));
    EXPECT_CALL(_observer, routeTimedOut(0U));
    advance(150U);
}

/**
 * Test that a routed request is rejected without reserving the route when no buffer is free.
 */
TEST_F(TransportRouterTest, RouteRequestWithoutFreeBufferIsRejected)
{
    std::vector<TransportMessage*> held;
    holdBuffers(held, TransportRouter::NUM_BUFFERS, 100U);
    EXPECT_EQ(ErrorCode::TPMSG_NO_MSG_AVAILABLE, requestBuffer(BUS_ETH, TESTER, NODE_A, 100U));
    EXPECT_EQ(RouteState::IDLE, _router.routeState(0U));
    releaseAll(held);
}

/**
 * Test that releasing one tester keeps the routes and the functional window of another
 * tester, and that releasing the owner frees them.
 */
TEST_F(TransportRouterTest, ReleasingAnotherTesterKeepsItsRoutes)
{
    sendRequest(NODE_A, _canA); // SENDING
    TransportMessage* functional = message(BUS_ETH, TESTER, FUNCTIONAL, {0x3E, 0x80});
    EXPECT_CALL(_canA, send(_, _)).WillOnce(Return(TpError::TP_SEND_FAIL));
    EXPECT_CALL(_canB, send(_, _)).WillOnce(Return(TpError::TP_SEND_FAIL));
    EXPECT_CALL(_local, send(_, _)).WillOnce(Return(TpError::TP_OK));
    _router.messageReceived(BUS_ETH, *functional, &_tester);
    _router.releaseTransportMessage(*functional);
    _router.releaseTester(TESTER_2);
    EXPECT_EQ(RouteState::SENDING, _router.routeState(0U));
    _router.releaseTester(TESTER);
    EXPECT_EQ(RouteState::IDLE, _router.routeState(0U));
    advance(10U); // functional window already closed: nothing to do
}

// --- unsolicited and late responses ------------------------------------------------------------

/**
 * Test that a response from a node without an outstanding request is discarded and counted.
 */
TEST_F(TransportRouterTest, UnsolicitedResponseIsDiscardedAndCounted)
{
    TransportMessage* msg = nullptr;
    EXPECT_EQ(
        ErrorCode::TPMSG_NOT_RESPONSIBLE,
        _router.getTransportMessage(BUS_CAN_A, NODE_A, GW_TESTER, 2U, {}, msg));
    EXPECT_EQ(1U, _statistics.get(0U, RouteCounter::DISCARDED));
    // a complete message that arrives anyway is not forwarded either
    uint8_t buffer[8];
    TransportMessage late(buffer, sizeof(buffer));
    late.setSourceAddress(NODE_A);
    late.setTargetAddress(GW_TESTER);
    late.setPayloadLength(2U);
    EXPECT_EQ(ReceiveResult::RECEIVED_ERROR, _router.messageReceived(BUS_CAN_A, late, nullptr));
    EXPECT_EQ(2U, _statistics.get(0U, RouteCounter::DISCARDED));
}

/**
 * Test that releasing a tester (e.g. its DoIP connection closed) frees its routes at once.
 *
 * A later response of the node is discarded, and another tester can use the route right
 * away.
 */
TEST_F(TransportRouterTest, ReleasedTesterFreesItsRoutesAtOnce)
{
    TransportMessage* request = sendRequest(NODE_A, _canA);
    confirmDelivery(*request);
    _router.releaseTester(TESTER_2); // other tester: no effect
    EXPECT_EQ(RouteState::WAIT_RESPONSE, _router.routeState(0U));
    _router.releaseTester(TESTER);
    EXPECT_EQ(RouteState::IDLE, _router.routeState(0U));
    EXPECT_EQ(ErrorCode::TPMSG_NOT_RESPONSIBLE, requestBuffer(BUS_CAN_A, NODE_A, GW_TESTER, 2U));
    EXPECT_EQ(1U, _statistics.get(0U, RouteCounter::DISCARDED));
    // a new tester can use the route immediately
    TransportMessage* next = message(BUS_ETH, TESTER_2, NODE_A, {0x3E, 0x00});
    EXPECT_NE(nullptr, next);
    _router.releaseTransportMessage(*next);
}

// --- functional routing ------------------------------------------------------------------------

/**
 * Test the functional routing of a request and of the responses within the window.
 *
 * The request goes to the local server and once to each route bus. Every node's response
 * within the functional window reaches the tester; the copies are released when their
 * delivery is confirmed, and responses after the window are discarded.
 */
TEST_F(TransportRouterTest, FunctionalRequestGoesToLocalAndOncePerRouteBus)
{
    TransportMessage* request = message(BUS_ETH, TESTER, FUNCTIONAL, {0x3E, 0x00});
    ASSERT_NE(nullptr, request);
    std::vector<TransportMessage*> copies;
    auto const copy = [&](TransportMessage& msg, ITransportMessageProcessedListener*)
    {
        EXPECT_EQ(GW_TESTER, msg.getSourceId());
        EXPECT_EQ(FUNCTIONAL, msg.getTargetId());
        EXPECT_EQ(2U, msg.getPayloadLength());
        copies.push_back(&msg);
        return TpError::TP_OK;
    };
    ITransportMessageProcessedListener* copyListener = nullptr;
    EXPECT_CALL(_canA, send(_, _)).WillOnce(DoAll(SaveArg<1>(&copyListener), Invoke(copy)));
    EXPECT_CALL(_canB, send(_, _)).WillOnce(Invoke(copy));
    EXPECT_CALL(_local, send(Ref(*request), &_tester)).WillOnce(Return(TpError::TP_OK));
    EXPECT_EQ(
        ReceiveResult::RECEIVED_NO_ERROR, _router.messageReceived(BUS_ETH, *request, &_tester));
    ASSERT_EQ(2U, copies.size());
    EXPECT_EQ(1U, _statistics.get(RouterCounter::FUNCTIONAL_REQUESTS));

    // responses of every node within the window reach the tester
    EXPECT_CALL(_eth, send(_, _)).Times(3).WillRepeatedly(Return(TpError::TP_OK));
    EXPECT_CALL(_observer, routeResponded(_)).Times(3);
    EXPECT_EQ(ReceiveResult::RECEIVED_NO_ERROR, nodeResponse(BUS_CAN_A, NODE_A, {0x7E, 0x00}));
    EXPECT_EQ(TESTER, _responseTarget);
    EXPECT_EQ(ReceiveResult::RECEIVED_NO_ERROR, nodeResponse(BUS_CAN_A, NODE_B, {0x7E, 0x00}));
    EXPECT_EQ(ReceiveResult::RECEIVED_NO_ERROR, nodeResponse(BUS_CAN_B, NODE_C, {0x7E, 0x00}));

    // the copies are released when their delivery is confirmed
    for (TransportMessage* msg : copies)
    {
        copyListener->transportMessageProcessed(*msg, Processing::PROCESSED_NO_ERROR);
    }
    _router.releaseTransportMessage(*request);
    EXPECT_EQ(
        _router.freeBuffers(), TransportRouter::NUM_BUFFERS + TransportRouter::NUM_SMALL_BUFFERS);

    // after the window, responses are discarded
    advance(150U);
    EXPECT_EQ(ErrorCode::TPMSG_NOT_RESPONSIBLE, requestBuffer(BUS_CAN_A, NODE_A, GW_TESTER, 2U));
}

/**
 * Test that a functional request above the maximum functional length is rejected.
 */
TEST_F(TransportRouterTest, FunctionalRequestLargerThanTheLimitIsRejected)
{
    EXPECT_EQ(ErrorCode::TPMSG_SIZE_TOO_LARGE, requestBuffer(BUS_ETH, TESTER, FUNCTIONAL, 8U));
}

/**
 * Test that a functional copy that a bus refuses is released at once.
 */
TEST_F(TransportRouterTest, FunctionalCopyThatCannotBeSentIsReleased)
{
    TransportMessage* request = message(BUS_ETH, TESTER, FUNCTIONAL, {0x3E, 0x80});
    EXPECT_CALL(_canA, send(_, _)).WillOnce(Return(TpError::TP_SEND_FAIL));
    EXPECT_CALL(_canB, send(_, _)).WillOnce(Return(TpError::TP_QUEUE_FULL));
    EXPECT_CALL(_local, send(_, _)).WillOnce(Return(TpError::TP_OK));
    EXPECT_EQ(
        ReceiveResult::RECEIVED_NO_ERROR, _router.messageReceived(BUS_ETH, *request, &_tester));
    _router.releaseTransportMessage(*request);
    EXPECT_EQ(
        _router.freeBuffers(), TransportRouter::NUM_BUFFERS + TransportRouter::NUM_SMALL_BUFFERS);
}

/**
 * Test that a functional request reaches only the local server when no buffer is free for the
 * copies, and that each missing copy is counted.
 */
TEST_F(TransportRouterTest, FunctionalRequestWithoutFreeBufferReachesOnlyLocal)
{
    // hold every buffer except one full-size buffer for the request itself
    std::vector<TransportMessage*> held;
    holdBuffers(held, TransportRouter::NUM_SMALL_BUFFERS, 2U);
    holdBuffers(held, TransportRouter::NUM_BUFFERS - 1U, 100U);
    TransportMessage* request = message(BUS_ETH, TESTER, FUNCTIONAL, {0x3E, 0x00});
    ASSERT_NE(nullptr, request);
    EXPECT_CALL(_local, send(Ref(*request), &_tester)).WillOnce(Return(TpError::TP_OK));
    EXPECT_EQ(
        ReceiveResult::RECEIVED_NO_ERROR, _router.messageReceived(BUS_ETH, *request, &_tester));
    EXPECT_EQ(2U, _statistics.get(RouterCounter::NO_BUFFER)); // one per route bus
    _router.releaseTransportMessage(*request);
    releaseAll(held);
}

/**
 * Test that releasing a tester closes its functional window: later responses are discarded.
 */
TEST_F(TransportRouterTest, ReleasedTesterClosesItsFunctionalWindow)
{
    TransportMessage* request = message(BUS_ETH, TESTER, FUNCTIONAL, {0x3E, 0x80});
    ITransportMessageProcessedListener* copyListener = nullptr;
    TransportMessage* copyA                          = nullptr;
    EXPECT_CALL(_canA, send(_, _))
        .WillOnce(DoAll(
            SaveArg<1>(&copyListener),
            Invoke(
                [&](TransportMessage& m, ITransportMessageProcessedListener*)
                {
                    copyA = &m;
                    return TpError::TP_OK;
                })));
    EXPECT_CALL(_canB, send(_, _)).WillOnce(Return(TpError::TP_SEND_FAIL));
    EXPECT_CALL(_local, send(_, _)).WillOnce(Return(TpError::TP_OK));
    _router.messageReceived(BUS_ETH, *request, &_tester);
    _router.releaseTester(TESTER);
    EXPECT_EQ(ErrorCode::TPMSG_NOT_RESPONSIBLE, requestBuffer(BUS_CAN_A, NODE_A, GW_TESTER, 2U));
    copyListener->transportMessageProcessed(*copyA, Processing::PROCESSED_NO_ERROR);
    _router.releaseTransportMessage(*request);
}

// --- buffers and statistics ------------------------------------------------------------------

/**
 * Test that running out of full-size buffers is counted, small messages still get small
 * buffers, and a foreign buffer is ignored on release.
 */
TEST_F(TransportRouterTest, BuffersRunOutAndAreCounted)
{
    std::vector<TransportMessage*> held;
    holdBuffers(held, TransportRouter::NUM_BUFFERS, 100U);
    TransportMessage* msg = nullptr;
    EXPECT_EQ(
        ErrorCode::TPMSG_NO_MSG_AVAILABLE,
        _router.getTransportMessage(BUS_LOCAL, LOCAL, TESTER, 100U, {}, msg));
    EXPECT_EQ(1U, _statistics.get(RouterCounter::NO_BUFFER));
    // small messages still fit into the small buffers
    EXPECT_EQ(
        ErrorCode::TPMSG_OK, _router.getTransportMessage(BUS_LOCAL, LOCAL, TESTER, 2U, {}, msg));
    EXPECT_EQ(TransportRouter::SMALL_BUFFER_SIZE, msg->getMaxPayloadLength());
    held.push_back(msg);
    releaseAll(held);
    uint8_t foreign[4];
    TransportMessage notOurs(foreign, sizeof(foreign));
    _router.releaseTransportMessage(notOurs); // ignored
    EXPECT_EQ(
        _router.freeBuffers(), TransportRouter::NUM_BUFFERS + TransportRouter::NUM_SMALL_BUFFERS);
}

/**
 * Test that counters saturate at 0xFFFF, ignore invalid route indices and are cleared by
 * reset().
 */
TEST_F(TransportRouterTest, StatisticsSaturateAndReset)
{
    TransportRouterStatistics statistics;
    for (uint32_t i = 0U; i < 0x10005U; ++i)
    {
        statistics.count(1U, RouteCounter::REQUESTS);
    }
    statistics.count(TransportRouterStatistics::MAX_ROUTES, RouteCounter::REQUESTS); // ignored
    statistics.count(RouterCounter::LOCAL_REQUESTS);
    EXPECT_EQ(0xFFFFU, statistics.get(1U, RouteCounter::REQUESTS));
    EXPECT_EQ(0U, statistics.get(TransportRouterStatistics::MAX_ROUTES, RouteCounter::REQUESTS));
    EXPECT_EQ(1U, statistics.get(RouterCounter::LOCAL_REQUESTS));
    statistics.reset();
    EXPECT_EQ(0U, statistics.get(1U, RouteCounter::REQUESTS));
    EXPECT_EQ(0U, statistics.get(RouterCounter::LOCAL_REQUESTS));
}

/**
 * Test that the router works and counts timeouts when no observer is set.
 */
TEST_F(TransportRouterTest, RouterWorksWithoutObserver)
{
    _router.setObserver(nullptr);
    TransportMessage* request = sendRequest(NODE_A, _canA);
    confirmDelivery(*request);
    advance(200U);
    EXPECT_EQ(1U, _statistics.get(0U, RouteCounter::TIMEOUTS));
}

/**
 * Test that the tester table replaces its oldest entry when more testers than entries send
 * requests.
 */
TEST_F(TransportRouterTest, TesterTableReplacesTheOldestTester)
{
    // more testers than table entries: the oldest is replaced
    for (uint16_t i = 0U; i <= TransportRouter::MAX_TESTERS; ++i)
    {
        TransportMessage* request
            = message(BUS_ETH, static_cast<uint16_t>(0x0EA0U + i), LOCAL, {0x3E, 0x00});
        EXPECT_CALL(_local, send(_, _)).WillOnce(Return(TpError::TP_OK));
        _router.messageReceived(BUS_ETH, *request, &_tester);
        _router.releaseTransportMessage(*request);
    }
    TransportMessage* toFirst = message(BUS_LOCAL, LOCAL, 0x0EA0U, {0x7E, 0x00});
    EXPECT_EQ(
        ReceiveResult::RECEIVED_ERROR, _router.messageReceived(BUS_LOCAL, *toFirst, &_nodeSide));
    _router.releaseTransportMessage(*toFirst);
    TransportMessage* toLast = message(
        BUS_LOCAL,
        LOCAL,
        static_cast<uint16_t>(0x0EA0U + TransportRouter::MAX_TESTERS),
        {0x7E, 0x00});
    EXPECT_CALL(_eth, send(_, _)).WillOnce(Return(TpError::TP_OK));
    EXPECT_EQ(
        ReceiveResult::RECEIVED_NO_ERROR, _router.messageReceived(BUS_LOCAL, *toLast, &_nodeSide));
    _router.releaseTransportMessage(*toLast);
}

} // namespace
