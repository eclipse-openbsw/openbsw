// Copyright 2024 Accenture.

#pragma once

#include <cstdint>

#ifdef __cplusplus
extern "C"
{
#endif
uint32_t softwareSystemResetCalled;

void softwareSystemReset() { ++softwareSystemResetCalled; }

void softwareDestructiveReset(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
