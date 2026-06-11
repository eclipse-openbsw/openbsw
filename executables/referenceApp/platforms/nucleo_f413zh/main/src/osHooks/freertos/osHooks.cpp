// Copyright 2026 An Dao
//
// SPDX-License-Identifier: Apache-2.0

#include "FreeRTOS.h"
#include "mcu/mcu.h"
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

// Feeds the IWDG on every tick. The IWDG may already be running from a
// previous firmware image (it cannot be disabled, only fed). This feed runs
// in addition to the SafetyManager's cyclic watchdog service.
void vApplicationTickHook(void) { IWDG->KR = 0xAAAAU; }
}
