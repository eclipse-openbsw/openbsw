/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "arch/sys_arch.h"

#include <bsp/SystemTime.h>
#include <interrupts/suspendResumeAllInterrupts.h>

extern "C"
{
void sys_init(void) {}

#ifdef PLATFORM_SUPPORT_ETHERNET
sys_prot_t sys_arch_protect(void) { return getOldIntEnabledStatusValueAndSuspendAllInterrupts(); }

void sys_arch_unprotect(sys_prot_t pval) { resumeAllInterrupts(pval); }

uint32_t auto_ip_create_seed_addr(void) { return 0; }

uint32_t sys_now(void) { return getSystemTimeMs32Bit(); }

#endif
}
