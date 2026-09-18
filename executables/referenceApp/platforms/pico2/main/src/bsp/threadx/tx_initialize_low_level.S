/**************************************************************************
 * Copyright (c) 2026 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 **************************************************************************/

/*
 * Low level ThreadX initialization for the pico2 board.
 *
 * The Cortex-M33 port of ThreadX does not ship a usable tx_initialize_low_level:
 * it hardcodes the SysTick reload value, refers to symbols this platform does
 * not define, and never gives the application a chance to set up its own
 * interrupts. This board specific version is used instead, mirroring what the
 * Cortex-M4 platforms in this repository do.
 */

#include "async/Config.h"

    .global     _tx_thread_system_stack_ptr
    .global     _tx_initialize_unused_memory
    .global     _tx_timer_interrupt
    .global     __vectors
    .global     __tx_SysTickHandler
    .extern     setupApplicationsIsr

@ Must match SYS_CLK_HZ of the vendored pico-sdk for the Cortex-M33, which is what
@ the pico-sdk runtime clock initialization actually programs the PLL to.
SYSTEM_CLOCK      =   150000000
SYSTICK_CYCLES    =   ((SYSTEM_CLOCK / ASYNC_CONFIG_TICK_IN_US) - 1)

    .text 32
    .align 4
    .syntax unified

@ VOID _tx_initialize_low_level(VOID)
    .global  _tx_initialize_low_level
    .thumb_func
_tx_initialize_low_level:
@
@    /* Disable interrupts during ThreadX initialization.  */
@
    CPSID   i
@
@    /* The vector table offset register is already programmed by the pico-sdk
@       crt0, which owns the vector table on this platform, so it is not
@       touched here.  */
@
@    /* Set system stack pointer from vector value.  */
@
    LDR     r0, =_tx_thread_system_stack_ptr        @ Build address of system stack pointer
    LDR     r1, =__vectors                          @ Pickup address of vector table
    LDR     r1, [r1]                                @ Pickup reset stack pointer
    STR     r1, [r0]                                @ Save system stack pointer
@
@    /* Enable the cycle count register.  */
@
    LDR     r0, =0xE0001000                         @ Build address of DWT register
    LDR     r1, [r0]                                @ Pickup the current value
    ORR     r1, r1, #1                              @ Set the CYCCNTENA bit
    STR     r1, [r0]                                @ Enable the cycle count register
@
@    /* Configure SysTick for the configured tick rate.  */
@
    MOV     r0, #0xE000E000                         @ Build address of NVIC registers
    LDR     r1, =SYSTICK_CYCLES
    STR     r1, [r0, #0x14]                         @ Setup SysTick Reload Value
    MOV     r1, #0x7                                @ Build SysTick Control Enable Value
    STR     r1, [r0, #0x10]                         @ Setup SysTick Control
@
@    /* Configure handler priorities.  */
@
    LDR     r1, =0x00000000                         @ Rsrv, UsgF, BusF, MemM
    STR     r1, [r0, #0xD18]                        @ Setup System Handlers 4-7 Priority Registers

    LDR     r1, =0xFF000000                         @ SVCl, Rsrv, Rsrv, Rsrv
    STR     r1, [r0, #0xD1C]                        @ Setup System Handlers 8-11 Priority Registers
                                                    @ Note: SVC must be lowest priority, which is 0xFF

    LDR     r1, =0x40FF0000                         @ SysT, PnSV, Rsrv, DbgM
    STR     r1, [r0, #0xD20]                        @ Setup System Handlers 12-15 Priority Registers
                                                    @ Note: PnSV must be lowest priority, which is 0xFF
@
@   /* Configure custom reference app ISRs */
@
    PUSH    {lr}
    BL      setupApplicationsIsr
    POP     {lr}
@
@    /* Return to caller.  */
@
    BX      lr

@ /* System Tick timer interrupt handler */
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

@
@    /* The pico-sdk crt0 owns the vector table and refers to the scheduler
@       exceptions as isr_svcall / isr_pendsv / isr_systick, for which it only
@       provides weak defaults. Alias them onto the ThreadX handlers so they
@       override those weak symbols at link time. Without this the SVC, PendSV
@       and SysTick exceptions are dispatched to the SDK default handler and no
@       context switch or tick ever reaches ThreadX.  */
