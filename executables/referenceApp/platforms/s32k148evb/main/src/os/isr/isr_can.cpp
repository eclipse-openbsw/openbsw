// Copyright 2024 Accenture.

#include "commonDebug.h"

extern "C"
{
extern void call_can_isr_RX();
extern void call_can_isr_TX();

void CAN0_ORed_0_15_MB_IRQHandler() { call_can_isr_RX(); }

void CAN0_ORed_16_31_MB_IRQHandler() { call_can_isr_TX(); }

} /* extern "C" */
