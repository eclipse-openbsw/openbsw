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

// Bring in size_t
#ifdef __cplusplus
#include <cstddef> // IWYU pragma: export
#else
#include <stddef.h> // IWYU pragma: export
#endif              //__cplusplus

#include "platform/config.h" // IWYU pragma: export

#if defined(HAS_CSTDINT_H_)
#include <cstdint>
#elif defined(HAS_STDINT_H_)
#include <stdint.h> // IWYU pragma: export
#endif

// In case some compilers do not define these values
// we define them here.
#ifndef UINTMAX_MAX
#define UINTMAX_MAX 0xffffffffffffffffULL
#endif // UINTMAX_MAX

#ifndef INTMAX_MAX
#define INTMAX_MAX 0x7fffffffffffffffULL
#endif // INTMAX_MAX
