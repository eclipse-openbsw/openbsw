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
void FTFC_Fault_IRQHandler()
{
    ::etl::println("FTFC_Fault_IRQHandler");
    // uncorrectable double bit ECC fault was detected in Flash
    etl::infinite_loop();
}

} // extern "C"
