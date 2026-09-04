/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "reset/softwareSystemReset.h"

#include <safeMemory/SafeMemory.h>

#include <etl/infinite_loop.h>

#include <cstdio>

extern "C"
{
void HardFault_Handler()
{
    // WARNING:
    // Do NOT modify this function if possible. Use HardFault_Handler_Final instead.
    // Refer to hardFaultHandler documentation for details.
    printf("HardFault_Handler\r\n");
#ifndef UNIT_TEST
    asm volatile("b customHardFaultHandler");
#else
    etl::infinite_loop();
#endif
}

void HardFault_Handler_Final()
{
    printf("HardFault_Handler_Final\r\n");
    ::safety::safe_memory::checkEccErrors();
    softwareSystemReset();
    etl::infinite_loop();
}

void BusFault_Handler()
{
    printf("BusFault_Handler\r\n");
    softwareSystemReset();
    etl::infinite_loop();
}

void UsageFault_Handler()
{
    printf("UsageFault_Handler\r\n");
    softwareSystemReset();
    etl::infinite_loop();
}

} // extern "C"
