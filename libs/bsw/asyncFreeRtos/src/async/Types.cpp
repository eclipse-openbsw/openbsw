/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "async/AsyncBinding.h"

namespace async
{
TimeoutType::TimeoutType() : _runnable(nullptr), _context(0) {}

void TimeoutType::cancel() { AsyncBindingType::AdapterType::cancel(*this); }

void TimeoutType::expired()
{
    RunnableType* const runnable = _runnable;
    if (runnable != nullptr)
    {
        runnable->execute();
    }
}

} // namespace async
