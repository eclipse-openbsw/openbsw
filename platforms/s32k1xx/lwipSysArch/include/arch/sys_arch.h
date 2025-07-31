// Copyright 2025 Accenture.

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
