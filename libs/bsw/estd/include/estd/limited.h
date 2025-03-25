// Copyright 2024 Accenture.

#pragma once

#include <limits>
#include <type_traits>

namespace estd
{
namespace internal
{
template<typename T, bool is_enum = std::is_enum<T>::value>
struct _limited_storage;

template<typename T>
struct _limited_storage<T, false>
{
    using storage_type = T;
};

template<typename T>
struct _limited_storage<T, true>
{
    using storage_type = typename std::underlying_type<T>::type;
};

template<typename T>
struct _limited_max
{
    using storage_type = typename _limited_storage<T>::storage_type;

    static constexpr storage_type LIMITED = std::numeric_limits<storage_type>::max();

    static bool is_limited(storage_type const value) { return value == LIMITED; }
};

} // namespace internal

template<typename T, typename PREDICATE = internal::_limited_max<T>>
struct limited
{
    static_assert(
        std::is_integral<T>::value || std::is_enum<T>::value,
        "estd::limited can only be used with integral or enum types");

    using storage_type = typename internal::_limited_storage<T>::storage_type;

    constexpr limited() : _value(PREDICATE::LIMITED) {}

    constexpr limited(T const value) : _value(static_cast<storage_type>(value)) {}

    T get() const { return static_cast<T>(_value); }

    T value_or(T const value) const { return has_value() ? get() : value; }

    bool has_value() const { return !PREDICATE::is_limited(_value); }

    void reset() { _value = PREDICATE::LIMITED; }

private:
    storage_type _value;
};

} // namespace estd

