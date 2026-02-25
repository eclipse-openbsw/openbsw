// Copyright 2025 BMW AG

#pragma once

#include <cstdint>

namespace middleware
{
namespace concurrency
{

/**
 * \brief Suspend all interrupts.
 * \details Platform-specific function that suspends all interrupts to ensure critical sections
 * are protected. This is used in conjunction with lock strategies for thread-safe operations.
 * The implementation must be provided for each platform integration.
 */
extern void suspendAllInterrupts();

/**
 * \brief Scoped lock for single-core protection.
 * \details RAII-style lock that protects critical sections within a single core by disabling
 * interrupts or using other single-core synchronization mechanisms. The lock is acquired in the
 * constructor and automatically released in the destructor, ensuring proper cleanup even in the
 * presence of exceptions.
 * The implementation must be provided for each platform integration.
 */
struct ScopedCoreLock
{
    /**
     * \brief Constructor that acquires the core lock.
     * \details Acquires the single-core lock by suspending interrupts or using other
     * synchronization mechanisms.
     */
    ScopedCoreLock();

    /**
     * \brief Destructor that releases the core lock.
     * \details Releases the single-core lock by restoring interrupts or releasing the
     * synchronization mechanism.
     */
    ~ScopedCoreLock();

    ScopedCoreLock(ScopedCoreLock const&)            = delete;
    ScopedCoreLock& operator=(ScopedCoreLock const&) = delete;
};

/**
 * \brief Scoped lock for ECU-wide (multi-core) protection.
 * \details RAII-style lock that protects critical sections across multiple cores in an ECU by
 * using hardware-supported spinlocks or other multi-core synchronization mechanisms. The lock is
 * acquired in the constructor and automatically released in the destructor, ensuring proper
 * cleanup even in the presence of exceptions.
 * The implementation must be provided for each platform integration.
 */
struct ScopedECULock
{
    /**
     * \brief Constructor that acquires the ECU lock.
     * \details Acquires the ECU-wide lock using the provided spinlock variable for multi-core
     * synchronization.
     *
     * \param lock pointer to the volatile spinlock variable
     */
    ScopedECULock(uint8_t volatile* lock);

    /**
     * \brief Destructor that releases the ECU lock.
     * \details Releases the ECU-wide lock, allowing other cores to acquire it.
     */
    ~ScopedECULock();

    ScopedECULock(ScopedECULock const&)            = delete;
    ScopedECULock& operator=(ScopedECULock const&) = delete;
};

} // namespace concurrency
} // namespace middleware
