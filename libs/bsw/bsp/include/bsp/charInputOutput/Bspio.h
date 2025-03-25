// Copyright 2024 Accenture.

#pragma once

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

