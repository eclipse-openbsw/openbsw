// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

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
