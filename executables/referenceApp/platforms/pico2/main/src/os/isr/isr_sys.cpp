/********************************************************************************
 * Copyright (c) 2024 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "reset/softwareSystemReset.h"

#include <etl/infinite_loop.h>
#include <etl/print.h>

extern "C"
{
void HardFault_Handler()
{
    // WARNING:
    // Do NOT modify this function if possible. Use HardFault_Handler_Final instead.
    // Refer to hardFaultHandler documentation for details.
    ::etl::println("HardFault_Handler");
#ifndef UNIT_TEST
    asm volatile("b customHardFaultHandler");
#else
    etl::infinite_loop();
#endif
}

void HardFault_Handler_Final()
{
    ::etl::println("HardFault_Handler_Final");
    softwareSystemReset();
    etl::infinite_loop();
}

void BusFault_Handler()
{
    ::etl::println("BusFault_Handler");
    softwareSystemReset();
    etl::infinite_loop();
}

void UsageFault_Handler()
{
    ::etl::println("UsageFault_Handler");
    softwareSystemReset();
    etl::infinite_loop();
}

} // extern "C"
