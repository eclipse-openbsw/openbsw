// Copyright 2024 Accenture.

#include "util/timeout/AbstractTimeout.h"

#include <async/Async.h>

namespace common
{
void AbstractTimeout::execute()
{
    if (_isActive)
    {
        _isActive = _isCyclic;
        expired(TimeoutExpiredActions(*this));
    }
}

bool AbstractTimeout::isActive() const { return _isActive; }

AbstractTimeout::TimeoutExpiredActions::TimeoutExpiredActions(AbstractTimeout& timeout)
: _timeout(timeout)
{}

AbstractTimeout::TimeoutExpiredActions::TimeoutExpiredActions(TimeoutExpiredActions const& rhs)
    = default;

void AbstractTimeout::TimeoutExpiredActions::reset() { reset(_timeout._time); }

void AbstractTimeout::TimeoutExpiredActions::reset(uint32_t const newTimeout)
{
    _timeout._asyncTimeout.cancel();
    _timeout._isActive = true;
    if (_timeout._isCyclic)
    {
        ::async::scheduleAtFixedRate(
            _timeout._context,
            _timeout,
            _timeout._asyncTimeout,
            newTimeout,
            ::async::TimeUnit::MILLISECONDS);
    }
    else
    {
        ::async::schedule(
            _timeout._context,
            _timeout,
            _timeout._asyncTimeout,
            newTimeout,
            ::async::TimeUnit::MILLISECONDS);
    }
}

void AbstractTimeout::TimeoutExpiredActions::cancel()
{
    _timeout._isActive = false;
    _timeout._asyncTimeout.cancel();
}

} // namespace common
