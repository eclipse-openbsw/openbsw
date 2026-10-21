/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <middleware/concurrency/MemoryFences.h>
#include <middleware/concurrency/SpinLock.h>
#include <atomic>

namespace middleware::concurrency
{
void spinLockOptimization() { asm volatile("pause" ::: "memory"); }

void atomicThreadFence() { std::atomic_thread_fence(std::memory_order_acq_rel); }
} // namespace middleware::concurrency
