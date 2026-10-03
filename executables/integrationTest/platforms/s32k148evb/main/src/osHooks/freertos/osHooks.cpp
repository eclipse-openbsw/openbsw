/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "FreeRTOS.h"
#include "task.h"

#include <cstdio>

extern "C"
{
void vApplicationStackOverflowHook(TaskHandle_t /* xTask */, char* /* pcTaskName */)
{
    printf("vApplicationStackOverflowHook\r\n");
}

void vIllegalISR()
{
    printf("vIllegalISR\r\n");
    for (;;)
        ;
}
}
