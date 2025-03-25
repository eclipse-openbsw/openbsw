// Copyright 2024 Accenture.

/**
 * Contains helper class to wrap a va_list started on the stack.
 * \file
 * \ingroup estl_utils
 */
#pragma once

#include <platform/estdint.h>

#include <cstdarg>

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

