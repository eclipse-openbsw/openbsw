/********************************************************************************
 * Copyright (c) 2025 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

// Platform integration provides ScopedCoreLock and ScopedECULock in the
// middleware::concurrency::integration namespace via lock_types.h.
// The concrete types will be provided at build time.
#include <middleware/concurrency/lock_types.h>

namespace middleware::concurrency
{

using ScopedCoreLock = ::middleware::concurrency::integration::ScopedCoreLock;
using ScopedECULock  = ::middleware::concurrency::integration::ScopedECULock;

} // namespace middleware::concurrency
