// Copyright 2026 An Dao
//
// SPDX-License-Identifier: Apache-2.0

#include "FreeRTOS.h"
#include "task.h"

#include <etl/infinite_loop.h>

extern "C"
{
void vApplicationStackOverflowHook(TaskHandle_t /* xTask */, char* /* pcTaskName */)
{
    etl::infinite_loop();
}

void vApplicationMallocFailedHook(void) { etl::infinite_loop(); }
}
