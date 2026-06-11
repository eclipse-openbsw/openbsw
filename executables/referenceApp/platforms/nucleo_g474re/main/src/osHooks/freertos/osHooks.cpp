// Copyright 2026 An Dao
//
// SPDX-License-Identifier: Apache-2.0

#include "FreeRTOS.h"
#include "task.h"

extern "C"
{
void vApplicationStackOverflowHook(TaskHandle_t /* xTask */, char* /* pcTaskName */)
{
    while (true) {}
}

void vApplicationMallocFailedHook(void)
{
    while (true) {}
}
}
