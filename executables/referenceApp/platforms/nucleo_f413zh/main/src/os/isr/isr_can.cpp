// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file isr_can.cpp
 * \brief CAN interrupt handlers for STM32F413 bxCAN — route to CanSystem
 */

extern "C"
{
extern void call_can_isr_RX();
extern void call_can_isr_TX();

void CAN1_RX0_IRQHandler(void) { call_can_isr_RX(); }

void CAN1_TX_IRQHandler(void) { call_can_isr_TX(); }
}
