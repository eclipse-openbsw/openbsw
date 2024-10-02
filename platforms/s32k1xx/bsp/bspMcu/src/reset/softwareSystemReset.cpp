// Copyright 2024 Accenture.

#include "reset/softwareSystemReset.h"

#include "mcu/mcu.h"

extern "C"
{
[[noreturn]] void softwareSystemReset(void) { NVIC_SystemReset(); }

void softwareDestructiveReset(void) { softwareSystemReset(); }
}
