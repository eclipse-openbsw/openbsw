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
  * | |\/| | |/ _` |/ _` | |/ _ \ \ /\ / / _` | '__/ _ \
  * | |  | | | (_| | (_| | |  __/\ V  V / (_| | | |  __/
  * |_|  |_|_|\__,_|\__,_|_|\___| \_/\_/ \__,_|_|  \___|
  *
  * WARNING!
  * This file is generated. Do not edit manually.
  *
  */
#pragma once

#include <middleware/concurrency/LockStrategies.h>
#include <middleware/queue/Queue.h>

#include "middleware/core/Message.h"

namespace middleware::shm
{

template <size_t Size>
using MiddlewareQueue =
    queue::Queue<queue::QueueTraits<core::Message, Size, concurrency::ScopedECULock, uint8_t*>>;

inline constexpr size_t QUEUE_TO_CORE0_SIZE = 10U;
inline constexpr size_t QUEUE_TO_CORE1_SIZE = 10U;

/** Returns a pointer to the queue in shared memory. Queues are fully initialized by MemoryLayout::MemoryLayout(). */
MiddlewareQueue<QUEUE_TO_CORE0_SIZE>* getQueueToCore0();
MiddlewareQueue<QUEUE_TO_CORE1_SIZE>* getQueueToCore1();

}  // namespace middleware::shm
