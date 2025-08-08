// Copyright 2025 Accenture.

#include "commonDebug.h"

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
