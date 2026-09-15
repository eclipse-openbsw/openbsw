/********************************************************************************
 * Copyright (c) 2024 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "mcu/mcu.h"
#include "platform/estdint.h"

extern "C"
{
void cacheEnable(void) {}

void cacheDisable(void) {}

void cacheInvalidate(uint32_t const /* addr */, uint32_t const /* size */) {}

} // extern "C"
