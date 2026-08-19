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

#include <cstdint>

namespace middleware::concurrency::integration
{

/**
 * No-op scope guard for single-core lock — simulation stub.
 */
struct ScopedCoreLock
{
    ScopedCoreLock()  = default;
    ~ScopedCoreLock() = default;

    ScopedCoreLock(ScopedCoreLock const&)            = delete;
    ScopedCoreLock& operator=(ScopedCoreLock const&) = delete;
    ScopedCoreLock(ScopedCoreLock&&)                 = delete;
    ScopedCoreLock& operator=(ScopedCoreLock&&)      = delete;
};

/**
 * No-op scope guard for ECU-wide lock — simulation stub.
 */
struct ScopedECULock
{
    explicit ScopedECULock(uint8_t volatile*) {}

    ~ScopedECULock() = default;

    ScopedECULock(ScopedECULock const&)            = delete;
    ScopedECULock& operator=(ScopedECULock const&) = delete;
    ScopedECULock(ScopedECULock&&)                 = delete;
    ScopedECULock& operator=(ScopedECULock&&)      = delete;
};

} // namespace middleware::concurrency::integration
