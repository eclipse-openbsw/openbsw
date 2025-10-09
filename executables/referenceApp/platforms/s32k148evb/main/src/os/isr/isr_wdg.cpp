// Copyright 2024 Accenture.

extern "C"
{
void WDOG_EWM_IRQHandler() { asm volatile("b customHardFaultHandler"); }

} // extern "C"
