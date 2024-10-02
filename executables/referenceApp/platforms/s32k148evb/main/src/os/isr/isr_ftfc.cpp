// Copyright 2024 Accenture.

#include "commonDebug.h"

extern "C"
{
void FTFC_Fault_IRQHandler()
{
    printf("FTFC_Fault_IRQHandler\r\n");
    // uncorrectable double bit ECC fault was detected in Flash
    while (true) {}
}

} // extern "C"
