// Copyright 2025 Accenture.

#include "arch/sys_arch.h"

#include "bsp/SystemTime.h"

#include <cstdint>

extern "C"
{
sys_prot_t sys_arch_protect(void) { return 0; }

void sys_arch_unprotect(sys_prot_t /* pval */) {}

uint32_t auto_ip_create_seed_addr(void) { return 0; }

uint32_t sys_now(void) { return getSystemTimeMs32Bit(); }
}
