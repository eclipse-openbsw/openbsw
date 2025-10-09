// Copyright 2024 Accenture.

#include "FreeRTOS.h"
#include "task.h"

extern "C"
{
void vApplicationStackOverflowHook(TaskHandle_t /* xTask */, char* /* pcTaskName */)
{
    while (true)
        ;
}

} // extern "C"
