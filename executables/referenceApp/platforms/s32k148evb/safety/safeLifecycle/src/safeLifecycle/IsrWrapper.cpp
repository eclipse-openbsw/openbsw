/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "safeLifecycle/IsrWrapper.h"

extern "C"
{
SAFE_ISR(FTFC_Fault_IRQHandler);
SAFE_ISR(WDOG_EWM_IRQHandler);
SAFE_ISR(ERM_double_fault_IRQHandler);
SAFE_ISR(CAN0_ORed_0_15_MB_IRQHandler);
SAFE_ISR(CAN0_ORed_16_31_MB_IRQHandler);
}
