/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/rpc/ProxyMethodImpl.h"

#include "middleware/core/Future.h"
#include "middleware/core/LoggerApi.h"
#include "middleware/core/Message.h"
#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/types.h"
#include "middleware/logger/Logger.h"
#include "middleware/time/SystemTimerProvider.h"

#include <etl/algorithm.h>

#include <cstdint>

namespace middleware::rpc
{

void ProxyMethodImpl::freeAll()
{
    for (auto& future : _futures)
    {
        future.state     = core::Future::State::Invalid;
        future.requestId = core::INVALID_REQUEST_ID;
    }
    _currentRequestId = core::INVALID_REQUEST_ID;
}

core::HRESULT ProxyMethodImpl::cancelRequest(uint16_t const requestId)
{
    core::HRESULT ret          = core::HRESULT::InstanceNotFound;
    core::Future* activeFuture = ::etl::find_if(
        _futures.begin(),
        _futures.end(),
        [requestId](core::Future& future) { return future.requestId == requestId; });

    if (activeFuture != _futures.end())
    {
        activeFuture->state     = core::Future::State::Invalid;
        activeFuture->requestId = core::INVALID_REQUEST_ID;

        ret = core::HRESULT::Ok;
    }

    return ret;
}

uint16_t ProxyMethodImpl::getNewRequestId()
{
    if (_currentRequestId == core::INVALID_REQUEST_ID)
    {
        _currentRequestId = 0U;
    }
    return _currentRequestId++;
}

core::Future* ProxyMethodImpl::futureMatchingRequestId(core::Message const& msg)
{
    uint16_t const reqId = msg.getHeader().requestId;
    core::Future* future = ::etl::find_if(
        _futures.begin(),
        _futures.end(),
        [reqId](core::Future& future) {
            return ((future.requestId == reqId) && (future.state == core::Future::State::Pending));
        });

    if (future != _futures.end())
    {
        future->requestId = core::INVALID_REQUEST_ID;
        core::Future::State const newState
            = msg.isError() ? static_cast<core::Future::State>(msg.getErrorState())
                            : core::Future::State::Ready;
        future->state = newState;
        return future;
    }

    logger::logMessageSendingFailure(
        logger::LogLevel::Error, logger::Error::SendMessage, core::HRESULT::FutureNotFound, msg);
    return nullptr;
}

core::Future* ProxyMethodImpl::obtainFuture()
{
    core::Future* const future = ::etl::find_if(
        _futures.begin(),
        _futures.end(),
        [](core::Future& fut) { return fut.state == core::Future::State::Invalid; });

    if (future == _futures.end())
    {
        return nullptr;
    }

    future->callerTimestamp = ::middleware::time::getCurrentTimeInMs();
    future->requestId       = getNewRequestId();
    future->state           = core::Future::State::Pending;

    return future;
}

etl::expected<uint16_t, core::HRESULT> ProxyMethodImpl::callMethod(
    core::ProxyBase const& proxy, core::Message& msg, core::HRESULT allocationResult)
{
    uint16_t const requestId = msg.getHeader().requestId;
    if (allocationResult != core::HRESULT::Ok)
    {
        cancelRequest(requestId);
        return etl::unexpected<core::HRESULT>(etl::in_place, allocationResult);
    }

    core::HRESULT const sendResult = proxy.sendMessage(msg);
    if (sendResult != core::HRESULT::Ok)
    {
        core::MessagePayloadBuilder::deallocate(msg);
        static_cast<void>(cancelRequest(requestId));
        return etl::unexpected<core::HRESULT>(etl::in_place, sendResult);
    }

    return requestId;
}

void ProxyMethodImpl::updateTimeouts(
    uint32_t const timeout, etl::delegate<void(core::Future&)> const callback)
{
    uint32_t const now = ::middleware::time::getCurrentTimeInMs();
    for (auto& future : _futures)
    {
        if (future.state == core::Future::State::Pending)
        {
            if (now - future.callerTimestamp > timeout)
            {
                future.state     = core::Future::State::Timeout;
                future.requestId = core::INVALID_REQUEST_ID;
                callback(future);
                future.state = core::Future::State::Invalid;
            }
        }
    }
}

} // namespace middleware::rpc
