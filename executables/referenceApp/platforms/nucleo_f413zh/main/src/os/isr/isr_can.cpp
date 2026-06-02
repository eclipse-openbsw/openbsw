/********************************************************************************
 * Copyright (c) 2026 An Dao
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

void CAN1_RX0_IRQHandler(void) { call_can_isr_RX(); }

void CAN1_TX_IRQHandler(void) { call_can_isr_TX(); }
}
