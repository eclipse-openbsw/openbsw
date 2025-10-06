// Copyright 2025 Accenture.

#pragma once

// clang-format off
#define SAFE_ISR(func) \
void Safe_##func(void) \
{                      \
    SafePreIsrHook();  \
    func();            \
    SafePostIsrHook(); \
}

#define SAFE_ISR_DECL(func) \
void func(void);            \
void Safe_##func(void);
// clang-format on

extern "C"
{
void SafePreIsrHook(void);
void SafePostIsrHook(void);

SAFE_ISR_DECL(FTFC_Fault_IRQHandler);
SAFE_ISR_DECL(WDOG_EWM_IRQHandler);
SAFE_ISR_DECL(ERM_double_fault_IRQHandler);
SAFE_ISR_DECL(CAN0_ORed_0_15_MB_IRQHandler);
SAFE_ISR_DECL(CAN0_ORed_16_31_MB_IRQHandler);
}
