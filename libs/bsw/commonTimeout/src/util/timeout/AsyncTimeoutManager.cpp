// Copyright 2024 Accenture.

#include "util/timeout/AsyncTimeoutManager.h"

#include "async/Async.h"
#include "estd/assert.h"
#include "util/timeout/AbstractTimeout.h"

namespace common
{
AsyncTimeoutManager::AsyncTimeoutManager() : _context(::async::CONTEXT_INVALID) {}

void AsyncTimeoutManager::init(::async::ContextType const context) { _context = context; }

void AsyncTimeoutManager::init() {}

void AsyncTimeoutManager::shutdown() {}

ITimeoutManager2::ErrorCode
AsyncTimeoutManager::set(AbstractTimeout& timeout, uint32_t const time, bool const cyclic)
{
    if (timeout._isActive)
    {
        return ErrorCode::TIMEOUT_ALREADY_SET;
    }
    timeout._time     = time;
    timeout._isCyclic = cyclic;
    timeout._isActive = true;
    timeout._context  = _context;
    if (cyclic)
    {
        ::async::scheduleAtFixedRate(
            _context, timeout, timeout._asyncTimeout, time, ::async::TimeUnit::MILLISECONDS);
    }
    else
    {
        ::async::schedule(
            _context, timeout, timeout._asyncTimeout, time, ::async::TimeUnit::MILLISECONDS);
    }
    return ErrorCode::TIMEOUT_OK;
}

void AsyncTimeoutManager::cancel(AbstractTimeout& timeout)
{
    timeout._isActive = false;
    timeout._asyncTimeout.cancel();
}

} // namespace common
