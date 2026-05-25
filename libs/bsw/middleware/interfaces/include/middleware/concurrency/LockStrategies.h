// Copyright 2025 BMW AG

#pragma once

// Platform integration provides ScopedCoreLock and ScopedECULock in the
// middleware::concurrency::integration namespace via lock_types.h.
// The concrete types will be provided at build time.
#include <middleware/concurrency/lock_types.h>

namespace middleware::concurrency
{

/**
 * \brief Suspends all interrupts for the current core
 */
extern void suspendAllInterrupts();

using ScopedCoreLock = ::middleware::concurrency::integration::ScopedCoreLock;
using ScopedECULock  = ::middleware::concurrency::integration::ScopedECULock;

} // namespace middleware::concurrency
