/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
