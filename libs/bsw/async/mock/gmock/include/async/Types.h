/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup async
 */
#pragma once

#include "platform/estdint.h"

namespace async
{
using ContextType = uint8_t;

ContextType const CONTEXT_INVALID = 0xFFU;

class LockType
{
public:
    LockType();
    ~LockType();
};

class ModifiableLockType
{
public:
    ModifiableLockType();
    ~ModifiableLockType();
    void unlock();
    void lock();

private:
    bool _isLocked;
};

class RunnableType
{
public:
    virtual ~RunnableType() = default;
    virtual void execute()  = 0;
};

struct TimeUnit
{
    enum Type
    {
        MICROSECONDS = 1,
        MILLISECONDS = 1000,
        SECONDS      = 1000000
    };
};

using TimeUnitType = TimeUnit::Type;

class TimeoutType
{
public:
    void cancel();
};

} // namespace async
