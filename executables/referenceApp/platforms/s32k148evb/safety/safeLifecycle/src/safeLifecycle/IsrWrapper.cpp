// Copyright 2024 Accenture.

#include "safeLifecycle/IsrWrapper.h"

extern "C"
{
SAFE_ISR(FTFC_Fault_IRQHandler);
SAFE_ISR(WDOG_EWM_IRQHandler);
SAFE_ISR(ERM_double_fault_IRQHandler);
SAFE_ISR(CAN0_ORed_0_15_MB_IRQHandler);
SAFE_ISR(CAN0_ORed_16_31_MB_IRQHandler);
}
