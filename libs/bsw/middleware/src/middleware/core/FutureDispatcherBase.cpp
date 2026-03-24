// Copyright 2025 BMW AG

#include "middleware/core/FutureDispatcherBase.h"

#include "middleware/core/Future.h"
#include "middleware/core/LoggerApi.h"
#include "middleware/core/Message.h"
#include "middleware/core/types.h"
#include "middleware/logger/Logger.h"

#include <etl/algorithm.h>
#include <etl/optional.h>

#include <cstdint>

namespace middleware
{
namespace core
{

void FutureDispatcherBase::freeAll()
{
    for (auto& future : _futures)
    {
        future.state     = Future::State::Invalid;
        future.requestId = INVALID_REQUEST_ID;
    }
    _currentRequestId = INVALID_REQUEST_ID;
}

HRESULT FutureDispatcherBase::cancelRequest(uint16_t const requestId)
{
    HRESULT ret          = HRESULT::InstanceNotFound;
    Future* activeFuture = etl::find_if(
        _futures.begin(),
        _futures.end(),
        [requestId](Future& future) { return future.requestId == requestId; });

    if (activeFuture != _futures.end())
    {
        activeFuture->state     = Future::State::Invalid;
        activeFuture->requestId = INVALID_REQUEST_ID;

        ret = HRESULT::Ok;
    }

    return ret;
}

etl::optional<Future*> FutureDispatcherBase::obtainRequestId(uint16_t& requestId)
{
    etl::optional<Future*> ret{};
    Future* const future = etl::find_if(
        _futures.begin(),
        _futures.end(),
        [](Future& fut) { return fut.state == Future::State::Invalid; });

    if (future == _futures.end())
    {
        requestId = INVALID_REQUEST_ID;
        ret       = etl::nullopt;
    }
    else
    {
        auto const newRequestId = getRequestId();
        future->requestId       = newRequestId;
        future->state           = Future::State::Pending;
        requestId               = newRequestId;
        ret                     = future;
    }
    return ret;
}

uint16_t FutureDispatcherBase::getRequestId()
{
    if (_currentRequestId == INVALID_REQUEST_ID)
    {
        _currentRequestId = 0U;
    }
    return _currentRequestId++;
}

Future* FutureDispatcherBase::futureMatchingRequestId(Message const& msg)
{
    uint16_t const reqId = msg.getHeader().requestId;
    Future* future       = etl::find_if(
        _futures.begin(),
        _futures.end(),
        [reqId](Future& future)
        { return ((future.requestId == reqId) && (future.state == Future::State::Pending)); });

    if (future != _futures.end())
    {
        future->requestId            = INVALID_REQUEST_ID;
        Future::State const newState = msg.isError()
                                           ? static_cast<Future::State>(msg.getErrorState())
                                           : Future::State::Ready;
        future->state                = newState;
        return future;
    }

    logger::logMessageSendingFailure(
        logger::LogLevel::Error, logger::Error::SendMessage, HRESULT::FutureNotFound, msg);
    return nullptr;
}

void FutureDispatcherBase::updateTimeouts(uint32_t const now, uint32_t const timeout)
{
    for (auto& future : _futures)
    {
        if (future.state == Future::State::Pending)
        {
            if (now - future.callerTimestamp > timeout)
            {
                future.state      = Future::State::Timeout;
                future.requestId  = INVALID_REQUEST_ID;
                // Invoke callback with dummy message; state indicates timeout
                Message const msg = Message::createEvent(0U, 0U, 0U, 0U);
                _callback(msg, future);
                future.state = Future::State::Invalid;
            }
        }
    }
}

} // namespace core
} // namespace middleware
