// Copyright 2024 Accenture.

#pragma once

#include <etl/utility.h>

namespace util
{
namespace defer
{
namespace internal
{
// C++11 version of std::exchange for internal use.
template<typename T, typename U = T>
inline T exchange(T& obj, U&& new_val)
{
    T const old_val = ::etl::move(obj);
    obj             = ::etl::forward<U>(new_val);
    return old_val;
}
} // namespace internal

/**
 * Defer allows you to defer a function call until the surrounding function returns
 */
template<class F>
class Defer
{
public:
    static_assert(
        (!::etl::is_reference<F>::value) && (!::etl::is_const<F>::value)
            && (!::etl::is_volatile<F>::value),
        "Defer should store its callable by value");

    Defer(Defer const&)            = delete;
    Defer& operator=(Defer const&) = delete;
    Defer& operator=(Defer&&)      = delete;

    explicit Defer(F f) : _f{::etl::move(f)}, _invoke{true} {}

    Defer(Defer&& other)
    : _f{::etl::move(other._f)}, _invoke{internal::exchange(other._invoke, false)}
    {}

    ~Defer()
    {
        if (_invoke)
        {
            _f();
        }
    }

private:
    F _f;
    bool _invoke;
};

/**
 * Convenient way to use Defer
 */
template<class F>
Defer<F> defer(F&& f)
{
    return Defer<F>(::etl::forward<F>(f));
}
} // namespace defer
} // namespace util
