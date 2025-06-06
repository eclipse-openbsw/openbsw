// Copyright 2024 Accenture.

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
