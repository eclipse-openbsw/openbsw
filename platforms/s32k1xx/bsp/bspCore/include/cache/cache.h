// Copyright 2024 Accenture.

#ifndef GUARD_84083D77_EDFF_44FE_A650_2C42AB323B3E
#define GUARD_84083D77_EDFF_44FE_A650_2C42AB323B3E

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

#endif // extern "C"
