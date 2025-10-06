// Copyright 2025 Accenture.

#pragma once

#include <cstdint>

uint32_t getMachineStateRegisterValueAndSuspendAllInterrupts(void);
void resumeAllInterrupts(uint32_t oldMachineStateRegisterValue);
