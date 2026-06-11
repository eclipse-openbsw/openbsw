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

extern "C"
{
void HardFault_Handler()
{
    // WARNING:
    // Do NOT modify this function if possible. Use HardFault_Handler_Final instead.
    // Refer to hardFaultHandler documentation for details.
#ifndef UNIT_TEST
    asm volatile("b customHardFaultHandler");
#else
    while (true) {}
#endif
}

void HardFault_Handler_Final() { softwareSystemReset(); }

} // extern "C"
