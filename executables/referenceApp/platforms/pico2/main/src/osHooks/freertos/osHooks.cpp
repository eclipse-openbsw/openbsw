/********************************************************************************
 * Copyright (c) 2024 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "FreeRTOS.h"
#include "task.h"

#include <etl/infinite_loop.h>
#include <etl/print.h>

extern "C"
{
void vApplicationStackOverflowHook(TaskHandle_t /* xTask */, char* /* pcTaskName */)
{
    ::etl::println("vApplicationStackOverflowHook");
}

void vIllegalISR()
{
    ::etl::println("vIllegalISR");
    ::etl::infinite_loop();
}
}
