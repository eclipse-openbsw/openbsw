// Copyright 2024 Accenture.

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
