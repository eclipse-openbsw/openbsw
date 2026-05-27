/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <cstdio>

extern "C"
{
void ERM_double_fault_IRQHandler()
{
    printf("ERM_double_fault_IRQHandler\r\n");
    // uncorrectable double bit ECC fault was detected in SRAM
    while (true) {}
}

} // extern "C"
