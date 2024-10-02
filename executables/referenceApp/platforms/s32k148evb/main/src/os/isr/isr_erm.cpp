// Copyright 2024 Accenture.

#include "commonDebug.h"

extern "C"
{
void ERM_double_fault_IRQHandler()
{
    printf("ERM_double_fault_IRQHandler\r\n");
    // uncorrectable double bit ECC fault was detected in SRAM
    while (true) {}
}

} // extern "C"
