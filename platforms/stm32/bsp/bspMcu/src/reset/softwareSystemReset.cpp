/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "reset/softwareSystemReset.h"

#include "mcu/mcu.h"

extern "C"
{
[[noreturn]] void softwareSystemReset(void) { NVIC_SystemReset(); }
}
