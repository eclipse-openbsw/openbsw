/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#include "MemoryLayout.h"
#include "middleware/shm/Config.h"

#include <etl/alignment.h>

namespace middleware::shm
{

// Deferred construction via typed_storage so the MemoryLayout (queues +
// allocator pools) is initialised at a well-defined point in startApp(),
// matching the pattern used for all other systems in app.cpp.
static ::etl::typed_storage<MemoryLayout> gMemoryLayout; // NOLINT(cert-err58-cpp)

void createMemoryLayout() { gMemoryLayout.create(); }

MemoryLayout& getMemoryLayout() { return *gMemoryLayout; }

} // namespace middleware::shm
