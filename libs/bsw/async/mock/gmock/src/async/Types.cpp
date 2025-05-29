// Copyright 2024 Accenture.

#include "async/Types.h"

#include "async/LockMock.h"
#include "async/TestContext.h"
#include "async/TimeoutMock.h"

#include <etl/singleton_base.h>

namespace async
{
using ::etl::singleton_base;

LockType::LockType()
{
    if (singleton_base<LockMock>::is_valid())
    {
        singleton_base<LockMock>::instance().lock();
    }
}

LockType::~LockType()
{
    if (singleton_base<LockMock>::is_valid())
    {
        singleton_base<LockMock>::instance().unlock();
    }
}

ModifiableLockType::ModifiableLockType() : _isLocked(false) { lock(); }

ModifiableLockType::~ModifiableLockType() { unlock(); }

void ModifiableLockType::unlock()
{
    if (_isLocked)
    {
        _isLocked = false;
        if (singleton_base<LockMock>::is_valid())
        {
            singleton_base<LockMock>::instance().unlock();
        }
    }
}

void ModifiableLockType::lock()
{
    if (!_isLocked)
    {
        _isLocked = true;
        if (singleton_base<LockMock>::is_valid())
        {
            singleton_base<LockMock>::instance().lock();
        }
    }
}

void TimeoutType::cancel()
{
    if ((!TestContext::cancelTimeout(*this)) && singleton_base<TimeoutMock>::is_valid())
    {
        singleton_base<TimeoutMock>::instance().cancel(*this);
    }
}

} // namespace async
