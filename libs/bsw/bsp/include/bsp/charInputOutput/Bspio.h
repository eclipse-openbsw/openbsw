// Copyright 2024 Accenture.

#ifndef GUARD_6917A99D_A9D7_454C_ACC5_23B96EF4211E
#define GUARD_6917A99D_A9D7_454C_ACC5_23B96EF4211E

#include <platform/estdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Embedded version of printf() .
 * \return On success, the total number of characters written is returned.
 *          On failure, zero is returned.
 */
uint8_t debug_printf(char const* format, ...);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* GUARD_6917A99D_A9D7_454C_ACC5_23B96EF4211E */
