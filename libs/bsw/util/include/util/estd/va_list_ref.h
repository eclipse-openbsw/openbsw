/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * Contains helper class to wrap a va_list started on the stack.
 * \file
 * \ingroup estl_utils
 */
#pragma once

#include <cstdarg>
#include <cstdint>

namespace estd
{
namespace internal
{
template<class T>
class _va_list_ref
{
public:
    using RefType = T;

    _va_list_ref(T& v) : _v(&v) {}

    T& get() const { return *_v; }

private:
    T* _v;
};

// On several ABIs (e.g. SysV x86-64) va_list is defined as a one-element C array. Handling that
// representation therefore requires C array types, so the diagnostic cannot be avoided here.
// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays)
template<class T, size_t N>
class _va_list_ref<T[N]>
{
public:
    using RefType = T (&)[N];

    _va_list_ref(T v[N]) : _v(v) {}

    RefType get() const { return reinterpret_cast<RefType>(*_v); }

private:
    T* _v;
};

// NOLINTEND(cppcoreguidelines-avoid-c-arrays)

} // namespace internal

/**
 * A simple helper class to store a reference to a va_list started with
 * a call of macro va_start.
 *
 * \note In C++98 there's the lack of the va_copy macro and thus only
 * simple references to va_list types can be used. Unfortunately the handling
 * of these references is platform-specific. Therefore more than one
 * implementation is provided and va_list_ref is a simple typedef
 */
using va_list_ref = internal::_va_list_ref<va_list>;

} // namespace estd
