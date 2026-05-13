// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file isr_can.cpp
 * \brief CAN interrupt handlers for STM32G474 FDCAN — route to CanSystem
 */

extern "C"
{
extern void call_can_isr_RX();
extern void call_can_isr_TX();

void FDCAN1_IT0_IRQHandler(void) { call_can_isr_RX(); }

void FDCAN1_IT1_IRQHandler(void) { call_can_isr_TX(); }
}
