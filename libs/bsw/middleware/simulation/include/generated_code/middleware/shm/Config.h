/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
/**
  *
  *  __  __ _     _     _ _
  * |  \/  (_) __| | __| | | _____      ____ _ _ __ ___
  * | |\/| | |/ _` |/ _` | |/ _ \\ \/ / / _` | '__/ _ \
  * | |  | | | (_| | (_| | |  __/\ V  V / (_| | | |  __/
  * |_|  |_|_|\__,_|\__,_|_|\___| \_/\_/ \__,_|_|  \___|
  *
  * WARNING!
  * This file is generated. Do not edit manually.
  *
  */
#pragma once

#include <cstdint>

#include "Allocator.h"
#include "shm/AllocatorsDefinitions.h"
#include "shm/QueueDefinitions.h"

namespace middleware::shm
{

// The declaration-order of the members here is intentional:
// mutexes first so their addresses are stable when used as default member initializer arguments
// for queues and allocators declared after them.
struct MemoryLayout
{
    uint8_t queueToCore0Mutex{0U};
    uint8_t queueToCore1Mutex{0U};
    uint8_t defaultAllocatorMutex{0U};
    MiddlewareQueue<QUEUE_TO_CORE0_SIZE> queueToCore0{&queueToCore0Mutex};
    MiddlewareQueue<QUEUE_TO_CORE1_SIZE> queueToCore1{&queueToCore1Mutex};
    message_allocator defaultAllocator{&defaultAllocatorMutex};
};

MemoryLayout& getMemoryLayout();

}  // namespace middleware::shm
