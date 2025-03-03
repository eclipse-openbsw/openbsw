// Copyright 2024 Accenture.

.syntax unified
.arch armv7-m



//------------------------------------------------------------------------------
// Unit constants
//------------------------------------------------------------------------------

// Dump location
.equ NO_INIT_RAM_START, 0x2000EC00
.equ NO_INIT_RAM_SIZE,  0x400
.equ DUMP_SIZE,         0x140
.equ DUMP_START,        NO_INIT_RAM_START + NO_INIT_RAM_SIZE - DUMP_SIZE

// Dump fields offsets
.equ DUMP_HANDLER_PSR,  0x000
.equ DUMP_HANDLER_LR,   0x004
.equ DUMP_ORIGIN_R0,    0x008
.equ DUMP_ORIGIN_R1,    0x00C
.equ DUMP_ORIGIN_R2,    0x010
.equ DUMP_ORIGIN_R3,    0x014
.equ DUMP_ORIGIN_R4,    0x018
.equ DUMP_ORIGIN_R5,    0x01C
.equ DUMP_ORIGIN_R6,    0x020
.equ DUMP_ORIGIN_R7,    0x024
.equ DUMP_ORIGIN_R8,    0x028
.equ DUMP_ORIGIN_R9,    0x02C
.equ DUMP_ORIGIN_R10,   0x030
.equ DUMP_ORIGIN_R11,   0x034
.equ DUMP_ORIGIN_R12,   0x038
.equ DUMP_ORIGIN_SP,    0x03C
.equ DUMP_ORIGIN_LR,    0x040
.equ DUMP_ORIGIN_PC,    0x044
.equ DUMP_ORIGIN_PSR,   0x048
.equ DUMP_STACK_START,  0x04C
.equ DUMP_STACK_END,    0x13C
.equ DUMP_CHECKSUM,     0x13C

// Stack frame offsets
.equ FRAME_R0,   0x00
.equ FRAME_R1,   0x04
.equ FRAME_R2,   0x08
.equ FRAME_R3,   0x0C
.equ FRAME_R12,  0x10
.equ FRAME_LR,   0x14
.equ FRAME_PC,   0x18
.equ FRAME_PSR,  0x1C
.equ FRAME_SIZE, 0x20



//------------------------------------------------------------------------------
// WARNING:
// R4-R11 must not be used before they are written to the dump, since they are
// not saved in stack. Stack must not be used.
//------------------------------------------------------------------------------

.text
.globl customHardFaultHandler

customHardFaultHandler:

    // Save flags for dump in advance
    mrs     R12, XPSR

    // Save PSR and LR
    ldr     R2, = DUMP_START
    str     R12, [R2, #DUMP_HANDLER_PSR]
    str     LR,  [R2, #DUMP_HANDLER_LR]

    // Save values of registers which are not framed
    str     R4,  [R2, #DUMP_ORIGIN_R4]
    str     R5,  [R2, #DUMP_ORIGIN_R5]
    str     R6,  [R2, #DUMP_ORIGIN_R6]
    str     R7,  [R2, #DUMP_ORIGIN_R7]
    str     R8,  [R2, #DUMP_ORIGIN_R8]
    str     R9,  [R2, #DUMP_ORIGIN_R9]
    str     R10, [R2, #DUMP_ORIGIN_R10]
    str     R11, [R2, #DUMP_ORIGIN_R11]

    // Analyze exception return code, calculate the value of SP which was
    // active at the moment of exception, and save it
    tst     LR, 4
    ite     eq
    mrseq   R1, MSP
    mrsne   R1, PSP
    add     R0, R1, FRAME_SIZE
    str     R0, [R2, #DUMP_ORIGIN_SP]

    // Save value of registers which are framed
    ldr     R0, [R1, #FRAME_R0]
    str     R0, [R2, #DUMP_ORIGIN_R0]
    ldr     R0, [R1, #FRAME_R1]
    str     R0, [R2, #DUMP_ORIGIN_R1]
    ldr     R0, [R1, #FRAME_R2]
    str     R0, [R2, #DUMP_ORIGIN_R2]
    ldr     R0, [R1, #FRAME_R3]
    str     R0, [R2, #DUMP_ORIGIN_R3]
    ldr     R0, [R1, #FRAME_R12]
    str     R0, [R2, #DUMP_ORIGIN_R12]
    ldr     R0, [R1, #FRAME_LR]
    str     R0, [R2, #DUMP_ORIGIN_LR]
    ldr     R0, [R1, #FRAME_PC]
    str     R0, [R2, #DUMP_ORIGIN_PC]
    ldr     R0, [R1, #FRAME_PSR]
    str     R0, [R2, #DUMP_ORIGIN_PSR]

    // Add some stack contents to dump
    add     R1, FRAME_SIZE
    add     R2, DUMP_STACK_START
    add     R3, R2, DUMP_STACK_END - DUMP_STACK_START
    1:
    ldr     R0, [R1], 4
    str     R0, [R2], 4
    cmp     R2, R3
    bne     1b

    // Calculate and save checksum
    mov     R1, 0
    ldr     R2, = DUMP_START
    ldr     R3, = DUMP_START + DUMP_CHECKSUM
    1:
    ldr     R0, [R2], 4
    add     R1, R0
    cmp     R2, R3
    bne     1b
    str     R1, [R3]

    // Complete HardFault handling
    b       HardFault_Handler_Final
