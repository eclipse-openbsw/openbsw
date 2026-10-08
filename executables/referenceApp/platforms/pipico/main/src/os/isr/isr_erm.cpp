/********************************************************************************
 * Copyright (c) 2024 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <etl/infinite_loop.h>
#include <etl/print.h>

extern "C"
{
void ERM_double_fault_IRQHandler()
{
    ::etl::println("ERM_double_fault_IRQHandler");
    // uncorrectable double bit ECC fault was detected in SRAM
    etl::infinite_loop();
}

} // extern "C"
