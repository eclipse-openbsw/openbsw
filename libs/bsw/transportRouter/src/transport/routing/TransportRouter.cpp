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

#include "transport/TpGatewayLogger.h"

#include <async/Async.h>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg): Logger API is variadic by design.

namespace transport
{
using ::util::logger::Logger;
using ::util::logger::TPGATEWAY;
using RouteCounter  = TransportRouterStatistics::RouteCounter;
using RouterCounter = TransportRouterStatistics::RouterCounter;

TransportRouter::TransportRouter(
    TransportRouterConfiguration const& configuration,
    TransportRouterStatistics& statistics,
    NowMsType const nowMs)
: _configuration(configuration)
, _statistics(statistics)
, _nowMs(nowMs)
, _observer(nullptr)
, _transportLayers()
, _routes()
, _testers()
, _nextTester(0U)
, _functionalCopyListener()
, _functionalActive(false)
, _functionalTester(0U)
, _functionalTesterBusId(INVALID_BUS)
, _functionalDeadline(0U)
, _buffers()
, _smallBuffers()
, _slots()
{
    for (size_t i = 0U; i < _routes.size(); ++i)
    {
        _routes[i].router = this;
        _routes[i].index  = static_cast<uint8_t>(i);
    }
    _functionalCopyListener.router = this;
    for (size_t i = 0U; i < _slots.size(); ++i)
    {
        Slot& slot  = _slots[i];
        slot.buffer = (i < NUM_BUFFERS) ? _buffers[i] : _smallBuffers[i - NUM_BUFFERS];
        slot.size   = (i < NUM_BUFFERS) ? BUFFER_SIZE : SMALL_BUFFER_SIZE;
        slot.locked = false;
        slot.route  = NO_ROUTE;
        slot.message.init(slot.buffer, slot.size);
    }
    for (Tester& tester : _testers)
    {
        tester = Tester{0U, INVALID_BUS};
    }
}

TransportRouter::ValidationError TransportRouter::validate(size_t& badRouteIndex) const
{
    badRouteIndex                           = INVALID_ROUTE;
    TransportRouterConfiguration const& cfg = _configuration;
    if (cfg.routes.empty())
    {
        return ValidationError::NO_ROUTES;
    }
    if (cfg.routes.size() > MAX_ROUTES)
    {
        return ValidationError::TOO_MANY_ROUTES;
    }
    if ((cfg.testerAddressMin > cfg.testerAddressMax) || isTester(cfg.localAddress)
        || isTester(cfg.functionalAddress) || (cfg.localAddress == cfg.functionalAddress))
    {
        return ValidationError::INVALID_TESTER_RANGE;
    }
    if ((cfg.maxFunctionalLength == 0U) || (cfg.maxFunctionalLength > SMALL_BUFFER_SIZE))
    {
        return ValidationError::INVALID_LENGTH;
    }
    if (cfg.transferTimeoutMs == 0U)
    {
        return ValidationError::INVALID_TIMING;
    }
    for (size_t i = 0U; i < cfg.routes.size(); ++i)
    {
        DiagnosticRoute const& route = cfg.routes[i];
        badRouteIndex                = i;
        if ((route.logicalAddress == cfg.localAddress)
            || (route.logicalAddress == cfg.functionalAddress)
            || (route.logicalAddress == cfg.gatewayTesterAddress) || isTester(route.logicalAddress)
            || (route.busId == cfg.localBusId))
        {
            return ValidationError::ADDRESS_CONFLICT;
        }
        if ((route.p2Ms == 0U) || (route.p2Ms > route.p2StarMs))
        {
            return ValidationError::INVALID_TIMING;
        }
        if ((route.maxLength == 0U) || (route.maxLength > BUFFER_SIZE))
        {
            return ValidationError::INVALID_LENGTH;
        }
        for (size_t j = 0U; j < i; ++j)
        {
            if (cfg.routes[j].logicalAddress == route.logicalAddress)
            {
                return ValidationError::DUPLICATE_ADDRESS;
            }
        }
    }
    badRouteIndex = INVALID_ROUTE;
    return ValidationError::NONE;
}

char const* TransportRouter::toString(ValidationError const error)
{
    switch (error)
    {
        case ValidationError::NONE:            return "ok";
        case ValidationError::NO_ROUTES:       return "no routes";
        case ValidationError::TOO_MANY_ROUTES: return "too many routes";
        case ValidationError::INVALID_TESTER_RANGE:
            return "tester range overlaps the local or functional address";
        case ValidationError::ADDRESS_CONFLICT:
            return "route overlaps the local, functional, gateway tester or tester addresses";
        case ValidationError::DUPLICATE_ADDRESS: return "duplicate route address";
        case ValidationError::INVALID_TIMING:    return "invalid P2/P2* or transfer budget";
        case ValidationError::INVALID_LENGTH:    return "invalid maximum length";
        default:                                 return "unknown";
    }
}

void TransportRouter::init()
{
    _transportLayers.clear();
    ::async::LockType const lock;
    _functionalActive = false;
    for (RouteContext& route : _routes)
    {
        route.state = RouteState::IDLE;
    }
}

void TransportRouter::shutdown() { _transportLayers.clear(); }

void TransportRouter::addTransportLayer(AbstractTransportLayer& transportLayer)
{
    for (AbstractTransportLayer const& layer : _transportLayers)
    {
        if (layer.getBusId() == transportLayer.getBusId())
        {
            Logger::error(
                TPGATEWAY,
                "TpLayer for bus %d must not be registered multiple times",
                transportLayer.getBusId());
            return;
        }
    }
    transportLayer.fProvidingListenerHelper.fpMessageListener = this;
    transportLayer.fProvidingListenerHelper.fpMessageProvider = this;
    _transportLayers.push_back(transportLayer);
}

void TransportRouter::removeTransportLayer(AbstractTransportLayer& transportLayer)
{
    if (!_transportLayers.contains_node(transportLayer))
    {
        return;
    }
    transportLayer.fProvidingListenerHelper.fpMessageListener = nullptr;
    transportLayer.fProvidingListenerHelper.fpMessageProvider = nullptr;
    _transportLayers.erase(transportLayer);
}

bool TransportRouter::isTester(uint16_t const address) const
{
    return (address >= _configuration.testerAddressMin)
           && (address <= _configuration.testerAddressMax);
}

size_t TransportRouter::routeIndex(uint16_t const logicalAddress) const
{
    for (size_t i = 0U; i < _configuration.routes.size(); ++i)
    {
        if (_configuration.routes[i].logicalAddress == logicalAddress)
        {
            return i;
        }
    }
    return INVALID_ROUTE;
}

size_t TransportRouter::routeFromNode(uint8_t const busId, uint16_t const sourceAddress) const
{
    size_t const index = routeIndex(sourceAddress);
    if ((index != INVALID_ROUTE) && (_configuration.routes[index].busId == busId))
    {
        return index;
    }
    return INVALID_ROUTE;
}

TransportRouter::RouteState TransportRouter::routeState(size_t const index) const
{
    ::async::LockType const lock;
    return (index < _configuration.routes.size()) ? _routes[index].state : RouteState::IDLE;
}

size_t TransportRouter::freeBuffers() const
{
    ::async::LockType const lock;
    size_t count = 0U;
    for (Slot const& slot : _slots)
    {
        if (!slot.locked)
        {
            ++count;
        }
    }
    return count;
}

void TransportRouter::learnTester(uint16_t const address, uint8_t const busId)
{
    ::async::LockType const lock;
    for (Tester& tester : _testers)
    {
        if (tester.address == address)
        {
            tester.busId = busId;
            return;
        }
    }
    _testers[_nextTester] = Tester{address, busId};
    _nextTester           = (_nextTester + 1U) % MAX_TESTERS;
}

uint8_t TransportRouter::testerBus(uint16_t const address) const
{
    ::async::LockType const lock;
    for (Tester const& tester : _testers)
    {
        if ((tester.busId != INVALID_BUS) && (tester.address == address))
        {
            return tester.busId;
        }
    }
    return INVALID_BUS;
}

ITransportMessageProvider::ErrorCode TransportRouter::allocate(
    uint16_t const size, uint8_t const route, TransportMessage*& pTransportMessage)
{
    // the smallest free buffer that fits
    Slot* best = nullptr;
    for (Slot& slot : _slots)
    {
        if ((!slot.locked) && (slot.size >= size)
            && ((best == nullptr) || (slot.size < best->size)))
        {
            best = &slot;
        }
    }
    if (best == nullptr)
    {
        _statistics.count(RouterCounter::NO_BUFFER);
        return ErrorCode::TPMSG_NO_MSG_AVAILABLE;
    }
    best->locked = true;
    best->route  = route;
    best->message.init(best->buffer, best->size);
    pTransportMessage = &best->message;
    return ErrorCode::TPMSG_OK;
}

ITransportMessageProvider::ErrorCode TransportRouter::getTransportMessage(
    uint8_t const srcBusId,
    uint16_t const sourceAddress,
    uint16_t const targetAddress,
    uint16_t const size,
    ::etl::span<uint8_t const> const& /* peek */,
    TransportMessage*& pTransportMessage)
{
    pTransportMessage = nullptr;
    ::async::LockType const lock;

    if (srcBusId == _configuration.localBusId)
    {
        // the local diagnostic server: responses and copies of functional requests
        return allocate(size, NO_ROUTE, pTransportMessage);
    }

    if (isTester(sourceAddress))
    {
        // The local diagnostic server may build its response in the request buffer, so
        // local and functional requests always get a full-size buffer.
        if (targetAddress == _configuration.localAddress)
        {
            return allocate(BUFFER_SIZE, NO_ROUTE, pTransportMessage);
        }
        if (targetAddress == _configuration.functionalAddress)
        {
            if (size > _configuration.maxFunctionalLength)
            {
                return ErrorCode::TPMSG_SIZE_TOO_LARGE;
            }
            return allocate(BUFFER_SIZE, NO_ROUTE, pTransportMessage);
        }
        size_t const index = routeIndex(targetAddress);
        if (index == INVALID_ROUTE)
        {
            _statistics.count(RouterCounter::UNKNOWN_TARGET);
            return ErrorCode::TPMSG_INVALID_TGT_ADDRESS;
        }
        if (size > _configuration.routes[index].maxLength)
        {
            _statistics.count(index, RouteCounter::NACK_TOO_LARGE);
            return ErrorCode::TPMSG_SIZE_TOO_LARGE;
        }
        if (_routes[index].state != RouteState::IDLE)
        {
            _statistics.count(index, RouteCounter::NACK_BUSY);
            return ErrorCode::TPMSG_NO_MSG_AVAILABLE;
        }
        ErrorCode const result = allocate(size, static_cast<uint8_t>(index), pTransportMessage);
        if (result == ErrorCode::TPMSG_OK)
        {
            _routes[index].state = RouteState::RESERVED;
        }
        return result;
    }

    size_t const index = routeFromNode(srcBusId, sourceAddress);
    if (index == INVALID_ROUTE)
    {
        return ErrorCode::TPMSG_NOT_RESPONSIBLE;
    }
    RouteContext& route = _routes[index];
    bool const pending
        = (route.state == RouteState::SENDING) || (route.state == RouteState::WAIT_RESPONSE);
    if ((!pending) && (!_functionalActive))
    {
        _statistics.count(index, RouteCounter::DISCARDED);
        return ErrorCode::TPMSG_NOT_RESPONSIBLE;
    }
    if (pending)
    {
        // P2 covers the start of the response; a segmented response may take longer.
        uint32_t const p2Star   = _configuration.routes[index].p2StarMs;
        uint32_t const transfer = _configuration.transferTimeoutMs;
        route.deadline          = _nowMs() + ((p2Star > transfer) ? p2Star : transfer);
    }
    return allocate(size, NO_ROUTE, pTransportMessage);
}

void TransportRouter::releaseTransportMessage(TransportMessage& transportMessage)
{
    ::async::LockType const lock;
    for (Slot& slot : _slots)
    {
        if (&slot.message == &transportMessage)
        {
            // released before it was forwarded, e.g. the tester side aborted the reception
            if ((slot.route != NO_ROUTE) && (_routes[slot.route].state == RouteState::RESERVED))
            {
                _routes[slot.route].state = RouteState::IDLE;
            }
            slot.locked = false;
            slot.route  = NO_ROUTE;
            return;
        }
    }
}

ITransportMessageListener::ReceiveResult TransportRouter::messageReceived(
    uint8_t const sourceBusId,
    TransportMessage& transportMessage,
    ITransportMessageProcessedListener* const pNotificationListener)
{
    if (sourceBusId == _configuration.localBusId)
    {
        // response of the local diagnostic server: back to the tester's bus
        uint8_t const busId = testerBus(transportMessage.getTargetId());
        if (busId == INVALID_BUS)
        {
            return ReceiveResult::RECEIVED_ERROR;
        }
        return forward(busId, transportMessage, pNotificationListener);
    }

    uint16_t const source = transportMessage.getSourceId();
    if (isTester(source))
    {
        learnTester(source, sourceBusId);
        uint16_t const target = transportMessage.getTargetId();
        if (target == _configuration.localAddress)
        {
            _statistics.count(RouterCounter::LOCAL_REQUESTS);
            return forward(_configuration.localBusId, transportMessage, pNotificationListener);
        }
        if (target == _configuration.functionalAddress)
        {
            return routeFunctional(sourceBusId, transportMessage, pNotificationListener);
        }
        size_t const index = routeIndex(target);
        if (index == INVALID_ROUTE)
        {
            return ReceiveResult::RECEIVED_ERROR;
        }
        return routeRequest(index, sourceBusId, transportMessage, pNotificationListener);
    }

    size_t const index = routeFromNode(sourceBusId, source);
    if (index == INVALID_ROUTE)
    {
        return ReceiveResult::RECEIVED_ERROR;
    }
    return routeNodeResponse(index, transportMessage, pNotificationListener);
}

ITransportMessageListener::ReceiveResult TransportRouter::routeRequest(
    size_t const index,
    uint8_t const testerBusId,
    TransportMessage& message,
    ITransportMessageProcessedListener* const listener)
{
    RouteContext& route = _routes[index];
    {
        ::async::LockType const lock;
        route.tester      = message.getSourceId();
        route.testerBusId = testerBusId;
        route.processed   = listener;
        route.state       = RouteState::SENDING;
        route.deadline    = _nowMs() + _configuration.transferTimeoutMs;
        _statistics.count(index, RouteCounter::REQUESTS);
    }
    message.setSourceAddress(_configuration.gatewayTesterAddress);
    ReceiveResult const result = forward(_configuration.routes[index].busId, message, &route);
    if (result != ReceiveResult::RECEIVED_NO_ERROR)
    {
        ::async::LockType const lock;
        message.setSourceAddress(route.tester);
        // the tester side keeps the buffer and releases it, which frees the route
        route.state = RouteState::RESERVED;
        _statistics.count(index, RouteCounter::TX_FAILURES);
    }
    return result;
}

ITransportMessageListener::ReceiveResult TransportRouter::routeFunctional(
    uint8_t const testerBusId,
    TransportMessage& message,
    ITransportMessageProcessedListener* const listener)
{
    {
        ::async::LockType const lock;
        _statistics.count(RouterCounter::FUNCTIONAL_REQUESTS);
        _functionalActive      = true;
        _functionalTester      = message.getSourceId();
        _functionalTesterBusId = testerBusId;
        _functionalDeadline    = _nowMs() + _configuration.functionalWindowMs;
    }
    // one copy per distinct route bus
    for (size_t i = 0U; i < _configuration.routes.size(); ++i)
    {
        uint8_t const busId = _configuration.routes[i].busId;
        bool firstOnBus     = true;
        for (size_t j = 0U; j < i; ++j)
        {
            if (_configuration.routes[j].busId == busId)
            {
                firstOnBus = false;
            }
        }
        if (!firstOnBus)
        {
            continue;
        }
        TransportMessage* copy = nullptr;
        {
            ::async::LockType const lock;
            (void)allocate(message.getPayloadLength(), NO_ROUTE, copy);
        }
        if (copy == nullptr)
        {
            continue;
        }
        copy->setSourceAddress(_configuration.gatewayTesterAddress);
        copy->setTargetAddress(_configuration.functionalAddress);
        copy->setPayloadLength(message.getPayloadLength());
        (void)copy->append(message.getPayload(), message.getPayloadLength());
        if (forward(busId, *copy, &_functionalCopyListener) != ReceiveResult::RECEIVED_NO_ERROR)
        {
            releaseTransportMessage(*copy);
        }
    }
    // the local diagnostic server is part of the functional group
    return forward(_configuration.localBusId, message, listener);
}

ITransportMessageListener::ReceiveResult TransportRouter::routeNodeResponse(
    size_t const index,
    TransportMessage& message,
    ITransportMessageProcessedListener* const listener)
{
    bool const pending  = isResponsePending(message);
    bool finished       = false;
    bool deliver        = true;
    uint16_t tester     = 0U;
    uint8_t testerBusId = INVALID_BUS;
    {
        ::async::LockType const lock;
        RouteContext& route = _routes[index];
        if ((route.state == RouteState::SENDING) || (route.state == RouteState::WAIT_RESPONSE))
        {
            tester      = route.tester;
            testerBusId = route.testerBusId;
            if (pending)
            {
                route.state    = RouteState::WAIT_RESPONSE;
                route.deadline = _nowMs() + _configuration.routes[index].p2StarMs;
            }
            else
            {
                route.state = RouteState::IDLE;
                finished    = true;
            }
        }
        else if (_functionalActive)
        {
            tester      = _functionalTester;
            testerBusId = _functionalTesterBusId;
            finished    = !pending;
        }
        else
        {
            deliver = false;
        }
        _statistics.count(
            index,
            (!deliver)  ? RouteCounter::DISCARDED
            : (pending) ? RouteCounter::PENDING
                        : RouteCounter::RESPONSES);
    }
    if (!deliver)
    {
        return ReceiveResult::RECEIVED_ERROR;
    }
    if (finished)
    {
        notify(index, true);
    }
    message.setTargetAddress(tester);
    return forward(testerBusId, message, listener);
}

void TransportRouter::RouteContext::transportMessageProcessed(
    TransportMessage& transportMessage, ProcessingResult const result)
{
    ITransportMessageProcessedListener* const original = processed;
    transportMessage.setSourceAddress(tester);
    router->routeProcessed(*this, result == ProcessingResult::PROCESSED_NO_ERROR);
    if (original != nullptr)
    {
        original->transportMessageProcessed(transportMessage, result);
    }
}

void TransportRouter::routeProcessed(RouteContext& route, bool const success)
{
    bool failed = false;
    {
        ::async::LockType const lock;
        if (route.state == RouteState::SENDING)
        {
            if (success)
            {
                route.state    = RouteState::WAIT_RESPONSE;
                route.deadline = _nowMs() + _configuration.routes[route.index].p2Ms;
            }
            else
            {
                route.state = RouteState::IDLE;
                failed      = true;
                _statistics.count(route.index, RouteCounter::TX_FAILURES);
            }
        }
    }
    if (failed)
    {
        notify(route.index, false);
    }
}

void TransportRouter::FunctionalCopyListener::transportMessageProcessed(
    TransportMessage& transportMessage, ProcessingResult const /* result */)
{
    router->releaseTransportMessage(transportMessage);
}

void TransportRouter::cyclic()
{
    uint32_t const now = _nowMs();
    for (size_t i = 0U; i < _configuration.routes.size(); ++i)
    {
        bool timedOut = false;
        {
            ::async::LockType const lock;
            RouteContext& route = _routes[i];
            if (((route.state == RouteState::SENDING) || (route.state == RouteState::WAIT_RESPONSE))
                && expired(now, route.deadline))
            {
                route.state = RouteState::IDLE;
                timedOut    = true;
                _statistics.count(i, RouteCounter::TIMEOUTS);
            }
        }
        if (timedOut)
        {
            Logger::info(
                TPGATEWAY,
                "no response from 0x%04x (%s)",
                _configuration.routes[i].logicalAddress,
                (_configuration.routes[i].name != nullptr) ? _configuration.routes[i].name : "");
            notify(i, false);
        }
    }
    ::async::LockType const lock;
    if (_functionalActive && expired(now, _functionalDeadline))
    {
        _functionalActive = false;
    }
}

void TransportRouter::releaseTester(uint16_t const testerAddress)
{
    ::async::LockType const lock;
    for (size_t i = 0U; i < _configuration.routes.size(); ++i)
    {
        RouteContext& route = _routes[i];
        if (((route.state == RouteState::SENDING) || (route.state == RouteState::WAIT_RESPONSE))
            && (route.tester == testerAddress))
        {
            route.state = RouteState::IDLE;
        }
    }
    if (_functionalActive && (_functionalTester == testerAddress))
    {
        _functionalActive = false;
    }
}

ITransportMessageListener::ReceiveResult TransportRouter::forward(
    uint8_t const busId,
    TransportMessage& message,
    ITransportMessageProcessedListener* const listener)
{
    for (AbstractTransportLayer& layer : _transportLayers)
    {
        if (layer.getBusId() == busId)
        {
            return (layer.send(message, listener) == AbstractTransportLayer::ErrorCode::TP_OK)
                       ? ReceiveResult::RECEIVED_NO_ERROR
                       : ReceiveResult::RECEIVED_ERROR;
        }
    }
    return ReceiveResult::RECEIVED_ERROR;
}

void TransportRouter::notify(size_t const index, bool const responded)
{
    if (_observer == nullptr)
    {
        return;
    }
    if (responded)
    {
        _observer->routeResponded(index);
    }
    else
    {
        _observer->routeTimedOut(index);
    }
}

bool TransportRouter::isResponsePending(TransportMessage const& message)
{
    return (message.getPayloadLength() >= 3U) && (message.getPayload()[0] == 0x7FU)
           && (message.getPayload()[2] == 0x78U);
}

} // namespace transport

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
