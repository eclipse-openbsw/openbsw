/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#ifndef ETL_PROFILE_H
#define ETL_PROFILE_H

#define ETL_TARGET_DEVICE_GENERIC
#define ETL_TARGET_OS_NONE

#define ETL_NO_STL

// When using clang, the headers are using the std::initializer_list and
// std::tuple (tuple_size) definitions even though we are configuring
// ETL_NO_STL in general
#include <initializer_list>
#include <tuple>

// Since at some points we are using std headers, we need to use this
// to prevent the ETL version from it conflicting with the
// std::initializer_list
#define ETL_FORCE_STD_INITIALIZER_LIST

// Don't rely on wchar_t support in the used libc++
#define ETL_NO_LIBC_WCHAR_H

// In unit tests, use C++ exceptions to check for ETL_ASSERT branches
#define ETL_THROW_EXCEPTIONS

#define ETL_CHRONO_HIGH_RESOLUTION_CLOCK_DURATION etl::chrono::nanoseconds
#define ETL_CHRONO_SYSTEM_CLOCK_DURATION          etl::chrono::microseconds
#define ETL_CHRONO_STEADY_CLOCK_DURATION          etl::chrono::seconds

#endif // ETL_PROFILE_H
