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

#include <middleware/memory/Aggregator.h>
#include <middleware/memory/Pool.h>

namespace middleware::memory
{

using message_allocator = Aggregator<::middleware::memory::Pool<10U, 64U>>;

} // namespace middleware::memory
