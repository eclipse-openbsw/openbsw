// Copyright 2025 BMW AG

#pragma once

#include <cstdint>

namespace middleware::concurrency
{

/**
 * Suspend all interrupts.
 * Platform-specific function that suspends all interrupts to ensure critical sections
 * are protected. This is used in conjunction with lock strategies for thread-safe operations.
 * The implementation must be provided for each platform integration.
 */
extern void suspendAllInterrupts();

/**
 * Scoped lock for single-core protection.
 * RAII-style lock that protects critical sections within a single core by disabling
 * interrupts or using other single-core synchronization mechanisms. The lock is acquired in the
 * constructor and automatically released in the destructor, ensuring proper cleanup even in the
 * presence of exceptions.
 * The implementation must be provided for each platform integration.
 */
struct ScopedCoreLock
{
    /** Acquires the single-core lock. */
    ScopedCoreLock();

    /** Releases the single-core lock. */
    ~ScopedCoreLock();

    ScopedCoreLock(ScopedCoreLock const&)            = delete;
    ScopedCoreLock& operator=(ScopedCoreLock const&) = delete;
};

/**
 * Scoped lock for ECU-wide (multi-core) protection.
 * RAII-style lock that protects critical sections across multiple cores in an ECU by
 * using hardware-supported spinlocks or other multi-core synchronization mechanisms. The lock is
 * acquired in the constructor and automatically released in the destructor, ensuring proper
 * cleanup even in the presence of exceptions.
 * The implementation must be provided for each platform integration.
 */
struct ScopedECULock
{
    /** Acquires the ECU-wide lock using \p lock. */
    ScopedECULock(uint8_t volatile* lock);

    /** Releases the ECU-wide lock. */
    ~ScopedECULock();

    ScopedECULock(ScopedECULock const&)            = delete;
    ScopedECULock& operator=(ScopedECULock const&) = delete;
};

} // namespace middleware::concurrency
