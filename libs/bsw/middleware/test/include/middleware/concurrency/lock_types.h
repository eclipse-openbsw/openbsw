#pragma once

#include <cstdint>

namespace middleware::concurrency::integration
{

/**
 * No-op scope guard for single-core lock — test/host stub.
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
 * No-op scope guard for ECU-wide lock — test/host stub.
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
