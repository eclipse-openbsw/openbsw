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

#include "Allocator.h"

namespace middleware::shm
{

using memory::message_allocator;

message_allocator& getDefaultAllocator();
volatile uint8_t* getDefaultAllocatorMutex();

}  // namespace middleware::shm
