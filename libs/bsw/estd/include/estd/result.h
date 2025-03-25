// Copyright 2024 Accenture.

/**
 * Contains estd::result class.
 * \file
 * \ingroup estl_time
 */
#pragma once

#include "estd/limited.h"
#include "estd/optional.h"
#include "estd/variant.h"

#include <type_traits>

namespace estd
{
template<typename T>
struct result_traits;

namespace internal
{
template<typename T, typename E>
struct _result_optimized
{
    // Detection of static member LIMITED
    template<typename S>
    static constexpr decltype(result_traits<S>::LIMITED, bool{}) is_optimized(int)
    {
        return result_traits<S>::LIMITED;
    }

    template<typename S>
    static constexpr bool is_optimized(...)
    {
        return false;
    }

    static constexpr bool value = std::is_same<T, void>::value && is_optimized<E>(int{});
};
} // namespace internal

/**
 * Type used for returning and propagating errors.
 * Semantics are similar to:
 *
 * - P0323R3 std::expected
 * - Result<T, E> in Rust
 * - Boost.Outcome
 *
 * @tparam T type stored when no error occurred; useful data
 * @tparam E type returned when error occurs
 */
template<
    typename T,
    typename E,
    bool OptimizedStorage = internal::_result_optimized<T, E>::value,
    bool IsVoid           = std::is_same<T, void>::value>
class result;

template<typename T, typename E>
class result<T, E, false, false>
{
    static_assert(
        (!std::is_convertible<T, E>::value) && (!std::is_convertible<E, T>::value),
        "Types used with result<T, E> should not be implicitly convertible to each other.");

    variant<T, E> _value;

public:
    using value_type = T;

    result(T const& value) : _value(value) {}

    result(E const& error) : _value(error) {}

    bool has_value() const { return _value.template is<T>(); }

    T& get() { return _value.template get<T>(); }

    T const& get() const { return _value.template get<T>(); }

    E& error() { return _value.template get<E>(); }

    E const& error() const { return _value.template get<E>(); }

    T* operator->() { return &_value.template get<T>(); }

    T const* operator->() const { return &_value.template get<T>(); }

    T& operator*() { return _value.template get<T>(); }

    T const& operator*() const { return _value.template get<T>(); }

    explicit operator bool() const { return has_value(); }

    T value_or(T const& alternative) const
    {
        if (_value.template is<T>())
        {
            return _value.template get<T>();
        }

        return alternative;
    }
};

/**
 * Special case: result<void, Error>
 */
template<typename T, typename E, bool OptimizedStorage>
class result<T, E, OptimizedStorage, true>
{
    using storage_type = typename std::conditional<OptimizedStorage, limited<E>, optional<E>>::type;

    storage_type _value;

public:
    result() : _value{} {}

    result(E const& error) : _value(error) {}

    bool has_value() const { return !_value.has_value(); }

    // Since this overload always operates with integral types,
    // return type is not a reference
    E error() const { return _value.get(); }
};
} // namespace estd

