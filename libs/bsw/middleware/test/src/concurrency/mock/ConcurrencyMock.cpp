/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "ConcurrencyMock.h"

#include "middleware/concurrency/MemoryFences.h"
#include "middleware/concurrency/SpinLock.h"

namespace middleware::concurrency
{

namespace
{
test::ConcurrencyMock* gConcurrencyMockPtr = nullptr;
} // namespace

namespace test
{

void setConcurrencyMock(ConcurrencyMock* const ptr) { gConcurrencyMockPtr = ptr; }

void unsetConcurrencyMock() { gConcurrencyMockPtr = nullptr; }

} // namespace test

// Without a registered mock both platform hooks are no-ops, as on a single-threaded host.
void atomicThreadFence()
{
    if (gConcurrencyMockPtr != nullptr)
    {
        gConcurrencyMockPtr->atomicThreadFence();
    }
}

void spinLockOptimization()
{
    if (gConcurrencyMockPtr != nullptr)
    {
        gConcurrencyMockPtr->spinLockOptimization();
    }
}

} // namespace middleware::concurrency
