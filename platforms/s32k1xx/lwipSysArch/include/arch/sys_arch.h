/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

// #include <interrupts/suspendResumeAllInterrupts.h>
#include <platform/estdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef uint32_t sys_prot_t;

sys_prot_t sys_arch_protect(void);
void sys_arch_unprotect(sys_prot_t pval);

#ifdef __cplusplus
}
#endif
