// Copyright 2024 Accenture.

#include "async/Types.h"

#include "async/LockMock.h"
#include "async/TestContext.h"
#include "async/TimeoutMock.h"

namespace async
{
using ::estd::singleton;

LockType::LockType()
{
    if (singleton<LockMock>::instantiated())
    {
        singleton<LockMock>::instance().lock();
    }
}

LockType::~LockType()
{
    if (singleton<LockMock>::instantiated())
    {
        singleton<LockMock>::instance().unlock();
    }
}

ModifiableLockType::ModifiableLockType() : _isLocked(false) { lock(); }

ModifiableLockType::~ModifiableLockType() { unlock(); }

void ModifiableLockType::unlock()
{
    if (_isLocked)
    {
        _isLocked = false;
        if (singleton<LockMock>::instantiated())
        {
            singleton<LockMock>::instance().unlock();
        }
    }
}

void ModifiableLockType::lock()
{
    if (!_isLocked)
    {
        _isLocked = true;
        if (singleton<LockMock>::instantiated())
        {
            singleton<LockMock>::instance().lock();
        }
    }
}

void TimeoutType::cancel()
{
    if ((!TestContext::cancelTimeout(*this)) && singleton<TimeoutMock>::instantiated())
    {
        singleton<TimeoutMock>::instance().cancel(*this);
    }
}

} // namespace async
