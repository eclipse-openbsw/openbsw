/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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

// NOLINTBEGIN(bugprone-exception-escape): This is just for testing purposes.
LockType::~LockType()
{
    if (singleton_base<LockMock>::is_valid())
    {
        singleton_base<LockMock>::instance().unlock();
    }
}

// NOLINTEND(bugprone-exception-escape)

ModifiableLockType::ModifiableLockType() : _isLocked(false) { lock(); }

// NOLINTNEXTLINE(bugprone-exception-escape): This is just for testing purposes.
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
