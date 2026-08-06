/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/


    .syntax unified
    .arch armv7-m

    .section .bootHeader, "a"
    .align 2
    .globl __bootHeader

__bootHeader:
    .long       __PATTERN1       /* 00h: */
    .long       __start+1          /* 04h: */
    .long       __PATTERN2       /* 08h: */
    .long       __PATTERN2       /* 0Ch: */

    .long       __PATTERN2       /* 10h:  __PATTERN3 */
    .long       __PATTERN2       /* 14h:  */
    .long       __PATTERN2       /* 18h:  */
    .long       __PATTERN2       /* 1Ch:  */

    .size    __bootHeader, . - __bootHeader
