@/***************************************************************************
@ * Copyright (c) 2024 Microsoft Corporation
@ *
@ * This program and the accompanying materials are made available under the
@ * terms of the MIT License which is available at
@ * https://opensource.org/licenses/MIT.
@ *
@ * SPDX-License-Identifier: MIT
@ **************************************************************************/
@
@ ThreadX low-level initialization for NUCLEO-G474RE (Cortex-M4F, 170 MHz)
@

#include "async/Config.h"

    .global     _tx_thread_system_stack_ptr
    .global     _tx_initialize_unused_memory
    .global     _tx_timer_interrupt
    .global     __main
    .global     g_pfnVectors
    .global     __tx_SysTickHandler
    .extern     setupApplicationsIsr

SYSTEM_CLOCK      =   170000000
SYSTICK_CYCLES    =   ((SYSTEM_CLOCK / ASYNC_CONFIG_TICK_IN_US) -1)

    .text 32
    .align 4
    .syntax unified

@ VOID   _tx_initialize_low_level(VOID)
    .global  _tx_initialize_low_level
    .thumb_func
_tx_initialize_low_level:

    CPSID   i

    @ Setup Vector Table Offset Register
    MOV     r0, #0xE000E000
    LDR     r1, =g_pfnVectors
    STR     r1, [r0, #0xD08]

    @ Set system stack pointer from vector value
    LDR     r0, =_tx_thread_system_stack_ptr
    LDR     r1, =g_pfnVectors
    LDR     r1, [r1]
    STR     r1, [r0]

    @ Enable the cycle count register (DWT)
    LDR     r0, =0xE0001000
    LDR     r1, [r0]
    ORR     r1, r1, #1
    STR     r1, [r0]

    @ Configure SysTick
    MOV     r0, #0xE000E000
    LDR     r1, =SYSTICK_CYCLES
    STR     r1, [r0, #0x14]                 @ SysTick Reload Value
    MOV     r1, #0x7                         @ Enable, interrupt, processor clock
    STR     r1, [r0, #0x10]                 @ SysTick Control

    @ Configure handler priorities
    LDR     r1, =0x00000000                 @ UsgF, BusF, MemM = 0
    STR     r1, [r0, #0xD18]

    LDR     r1, =0xFF000000                 @ SVCall = 0xFF (lowest)
    STR     r1, [r0, #0xD1C]

    LDR     r1, =0x40FF0000                 @ SysTick = 0x40, PendSV = 0xFF
    STR     r1, [r0, #0xD20]

    @ Configure platform-specific ISRs
    PUSH    {lr}
    BL      setupApplicationsIsr
    POP     {lr}

    BX      lr

@ SysTick handler — redirects to ThreadX timer interrupt
    .global  __tx_SysTickHandler
    .global  SysTick_Handler
    .thumb_func
__tx_SysTickHandler:
    .thumb_func
SysTick_Handler:
    PUSH    {r0, lr}
    BL      _tx_timer_interrupt
    POP     {r0, lr}
    BX      LR

    .section .text.SVC_Handler
    .global SVC_Handler
    .thumb_func
SVC_Handler:
    ldr r0, =Default_Handler
    bx  r0
