// Copyright 2024 Accenture.

#pragma once

#include "platform/estdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

void cacheEnable(void);
void cacheDisable(void);
void cacheInvalidate(uint32_t addr, uint32_t size);

#ifdef __cplusplus
}
#endif
