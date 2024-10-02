// Copyright 2024 Accenture.

#include "commonDebug.h"
#include "reset/softwareSystemReset.h"

extern "C"
{
void HardFault_Handler()
{
    // WARNING:
    // Do NOT modify this function if possible. Use HardFault_Handler_Final instead.
    // Refer to hardFaultHandler documentation for details.
    printf("HardFault_Handler\r\n");
#ifndef UNIT_TEST
    asm volatile("b customHardFaultHandler");
#else
    while (true) {}
#endif
}

void HardFault_Handler_Final()
{
    printf("HardFault_Handler_Final\r\n");
    softwareSystemReset();
    while (true) {}
}

void BusFault_Handler()
{
    printf("BusFault_Handler\r\n");
    softwareSystemReset();
    while (true) {}
}

void UsageFault_Handler()
{
    printf("UsageFault_Handler\r\n");
    softwareSystemReset();
    while (true) {}
}

} // extern "C"
