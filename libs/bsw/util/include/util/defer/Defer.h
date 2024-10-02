// Copyright 2024 Accenture.

#ifndef GUARD_53701C84_7FF2_11EB_842A_C34D68860C96
#define GUARD_53701C84_7FF2_11EB_842A_C34D68860C96

#include <utility>

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
    T const old_val = std::move(obj);
    obj             = std::forward<U>(new_val);
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
        (!std::is_reference<F>::value) && (!std::is_const<F>::value)
            && (!std::is_volatile<F>::value),
        "Defer should store its callable by value");

    Defer(Defer const&)            = delete;
    Defer& operator=(Defer const&) = delete;
    Defer& operator=(Defer&&)      = delete;

    explicit Defer(F f) : _f{std::move(f)}, _invoke{true} {}

    Defer(Defer&& other)
    : _f{std::move(other._f)}, _invoke{internal::exchange(other._invoke, false)}
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
    return Defer<F>(std::forward<F>(f));
}
} // namespace defer
} // namespace util

#endif // GUARD_53701C84_7FF2_11EB_842A_C34D68860C96
