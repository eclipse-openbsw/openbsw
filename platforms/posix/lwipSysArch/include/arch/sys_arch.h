// Copyright 2025 Accenture.

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

typedef int sys_prot_t;
sys_prot_t sys_arch_protect(void);
void sys_arch_unprotect(sys_prot_t pval);

#ifdef __cplusplus
}
#endif
