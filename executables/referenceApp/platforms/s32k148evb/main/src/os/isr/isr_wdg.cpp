// Copyright 2024 Accenture.

#include "commonDebug.h"

extern "C"
{
void WDOG_EWM_IRQHandler() { asm volatile("b customHardFaultHandler"); }

} // extern "C"
