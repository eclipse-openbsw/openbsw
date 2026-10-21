/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

namespace middleware::concurrency
{

// Platform integration shall provide atomicThreadFence() (link error otherwise).
// It shall provide acquire/release ordering (like std::memory_order_acq_rel) of
// MiddlewarePdu's non-atomic payload accesses against its sequence counter, as seen
// by all cores. A compiler barrier suffices only where the hardware already orders
// loads and stores that way (single core, x86); otherwise use a hardware barrier.

void atomicThreadFence();

} // namespace middleware::concurrency
