/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

// clang-format off
static inline __attribute__((always_inline))
void disableAllInterrupts(void)
{
asm(
"cpsid   i;"
"ISB;"
"DSB;"
"DMB;"
);
}
static inline __attribute__((always_inline))
void enableAllInterrupts(void)
{
asm (
"ISB;"
"DSB;"
"DMB;"
"cpsie   i;"
);
}

// clang-format on
