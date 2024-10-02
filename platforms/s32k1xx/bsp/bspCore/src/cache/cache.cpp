// Copyright 2024 Accenture.

#include "mcu/mcu.h"
#include "platform/estdint.h"

extern "C"
{
void cacheEnable(void)
{
    LMEM->PCCCR
        = LMEM_PCCCR_INVW0(1) | LMEM_PCCCR_INVW1(1) | LMEM_PCCCR_GO(1) | LMEM_PCCCR_ENCACHE(1);
}

void cacheDisable(void)
{
    LMEM->PCCCR
        = LMEM_PCCCR_INVW0(1) | LMEM_PCCCR_INVW1(1) | LMEM_PCCCR_GO(1) | LMEM_PCCCR_ENCACHE(0);
}

void cacheInvalidate(uint32_t const addr, uint32_t const size)
{
    LMEM->PCCCR
        = LMEM_PCCCR_INVW0(1) | LMEM_PCCCR_INVW1(1) | LMEM_PCCCR_GO(1) | LMEM_PCCCR_ENCACHE(1);
}

} // extern "C"
