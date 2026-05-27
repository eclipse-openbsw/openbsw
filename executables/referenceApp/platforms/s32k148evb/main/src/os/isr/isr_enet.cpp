/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

extern "C"
{
extern void enetRxIsr();
extern void enetTxIsr();

// extern void call_enet_isr_Error();

void ENET_TX_Buffer_IRQHandler() { enetTxIsr(); }

void ENET_RX_Buffer_IRQHandler() { enetRxIsr(); }

void ENET_PRE_IRQHandler()
{
    // call_enet_isr_Error();
}

} /* extern "C" */
