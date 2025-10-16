// Copyright 2025 Accenture.

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

#define ETL_FORCE_STD_INITIALIZER_LIST

#define ETL_NO_LIBC_WCHAR_H

#define ETL_USING_BUILTIN_MEMCPY  0
#define ETL_USING_BUILTIN_MEMMOVE 0
#define ETL_USING_BUILTIN_MEMSET  0
#define ETL_USING_BUILTIN_MEMCMP  0
#define ETL_USING_BUILTIN_MEMCHR  0

#define ETL_USE_ASSERT_FUNCTION

#endif // ETL_PROFILE_H
