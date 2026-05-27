/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

extern "C"
{
extern void call_can_isr_RX();
extern void call_can_isr_TX();

void CAN0_ORed_0_15_MB_IRQHandler() { call_can_isr_RX(); }

void CAN0_ORed_16_31_MB_IRQHandler() { call_can_isr_TX(); }

} /* extern "C" */
