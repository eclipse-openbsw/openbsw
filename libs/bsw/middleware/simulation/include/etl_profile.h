/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#pragma once

// Define tuple_size from clang headers consistently since
// ETL uses them for deduction guide
#include <tuple>

#include "etl/profiles/cpp17_no_stl.h"

// ETL_COMPILER_* will be autodetected correctly only if
// ETL_COMPILER_GENERIC (from default profile above) is not defined
#undef ETL_COMPILER_GENERIC

#define ETL_USE_TYPE_TRAITS_BUILTINS

#define ETL_TARGET_DEVICE_GENERIC
#define ETL_TARGET_OS_NONE

#define ETL_FORCE_STD_INITIALIZER_LIST
#define ETL_NO_LIBC_WCHAR_H

#define ETL_CHECK_PUSH_POP
#define ETL_LOG_ERRORS
#define ETL_VERBOSE_ERRORS
#define ETL_THROW_EXCEPTIONS
#define ETL_DEBUG

#define HUGE_VALL HUGE_VAL
