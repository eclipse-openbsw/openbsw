// Copyright 2024 Accenture.

/**
 * Contains estd::function, estd::closure.
 * \file
 * \ingroup estl_utils
 */
#ifndef GUARD_05AF1AC2_4439_47BF_8B23_BF54CFC4327C
#define GUARD_05AF1AC2_4439_47BF_8B23_BF54CFC4327C

#include "estd/type_traits.h"
#include "estd/type_utils.h"

namespace estd
{
struct none_t;

/**
 * A function object to encapsulate calling a global or member function.
 * The function can have any number of arguments and (optionally) a return value.
 *
 * Warning: estd::function acts as a function pointer/reference - if it points to a member function,
 * the object it is bound to needs to outlive the function object.
 */
template<typename>
struct function;

template<typename R, typename... Ps>
struct function<R(Ps...)>
{
    /** Create an empty function object. */
    function() { *this = create<&function::empty_function>(); }

    function(none_t const&) { *this = create<&function::empty_function>(); }

    function(function const& o) = default;

    function& operator=(function const&) = default;

    /**
     * Calls the underlying function object.
     *
     * \return The return of the underlying function object if it is specified.
     */
    R operator()(Ps... params) const { return (*_f)(_c, params...); }

    /**
     * Checks if the function has been set to a target.
     */
    explicit operator bool() const { return has_value(); }

    /**
     * Checks if two functions refer to the same target
     */
    bool operator==(function const& other) const { return (_f == other._f) && (_c == other._c); }

    /**
     * Checks if the function has been set to a target.
     */
    bool has_value() const { return _f != create<&function::empty_function>()._f; }

    /**
     * Creates a function object to a member function of typename T.
     *
     * \param obj The actual instance that contains the member function to call.
     * \return A new function object that encapsulates the member function.
     */
    template<typename T, R (T::*M)(Ps...)>
    static function create(T& t)
    {
        return function(&member_function_caller<T, M>, &t);
    }

    /**
     * Creates a function object to a const member function of class T.
     *
     * \param obj The actual const instance that contains the member function to call.
     * \return A new function object that encapsulates the member function.
     */
    template<typename T, R (T::*M)(Ps...) const>
    static function create(T const& t)
    {
        return function(&member_function_caller<T, M>, type_utils<T>::const_cast_to_void(&t));
    }

    /**
     * Creates a function object to a free function pointer.
     *
     * \return A new function object that encapsulates the free function.
     */
    template<R (*F)(Ps...)>
    static function create()
    {
        return function(&function_caller<F>);
    }

private:
    using type = R (*)(void*, Ps...);

    explicit function(type const f, void* const c = nullptr) : _f(f), _c(c) {}

    template<typename T, R (T::*M)(Ps...) const>
    static R member_function_caller(void* const c, Ps const... ps)
    {
        return (::estd::type_utils<T>::const_cast_from_void(c)->*M)(ps...);
    }

    template<typename T, R (T::*M)(Ps...)>
    static R member_function_caller(void* const c, Ps const... ps)
    {
        return (::estd::type_utils<T>::cast_from_void(c)->*M)(ps...);
    }

    template<R (*F)(Ps...)>
    static R function_caller(void*, Ps... ps)
    {
        return (*F)(ps...);
    }

    static R empty_function(Ps...) { return R(); }

    type _f;
    void* _c = nullptr;
};

/*
 * Closure support
 */
template<typename>
struct closure;

template<typename R, typename P0>
struct closure<R(P0)>
{
    using fct = function<R(P0)>;

    closure(fct const& f, P0 const p0) : _f(f), _p0(p0) {}

    R operator()() const { return _f(_p0); }

private:
    fct _f;
    P0 _p0;
};

template<typename R, typename P0, typename P1>
struct closure<R(P0, P1)>
{
    using fct = function<R(P0, P1)>;

    closure(fct const& f, P0 const p0, P1 const p1) : _f(f), _p0(p0), _p1(p1) {}

    R operator()() const { return _f(_p0, _p1); }

private:
    fct _f;
    P0 _p0;
    P1 _p1;
};

template<typename R, typename P0, typename P1, typename P2>
struct closure<R(P0, P1, P2)>
{
    using fct = function<R(P0, P1, P2)>;

    closure(fct const& f, P0 const p0, P1 const p1, P2 const p2) : _f(f), _p0(p0), _p1(p1), _p2(p2)
    {}

    R operator()() const { return _f(_p0, _p1, _p2); }

private:
    fct _f;
    P0 _p0;
    P1 _p1;
    P2 _p2;
};

template<typename R, typename P0, typename P1, typename P2, typename P3>
struct closure<R(P0, P1, P2, P3)>
{
    using fct = function<R(P0, P1, P2, P3)>;

    closure(fct const& f, P0 const p0, P1 const p1, P2 const p2, P3 const p3)
    : _f(f), _p0(p0), _p1(p1), _p2(p2), _p3(p3)
    {}

    R operator()() const { return _f(_p0, _p1, _p2, _p3); }

private:
    fct _f;
    P0 _p0;
    P1 _p1;
    P2 _p2;
    P3 _p3;
};

template<typename R, typename P0, typename P1, typename P2, typename P3, typename P4>
struct closure<R(P0, P1, P2, P3, P4)>
{
    using fct = function<R(P0, P1, P2, P3, P4)>;

    closure(fct const& f, P0 const p0, P1 const p1, P2 const p2, P3 const p3, P4 const p4)
    : _f(f), _p0(p0), _p1(p1), _p2(p2), _p3(p3), _p4(p4)
    {}

    R operator()() const { return _f(_p0, _p1, _p2, _p3, _p4); }

private:
    fct _f;
    P0 _p0;
    P1 _p1;
    P2 _p2;
    P3 _p3;
    P4 _p4;
};

template<typename>
struct binder1st;

template<typename R, typename B, typename... Ps>
struct binder1st<R(B, Ps...)>
{
    using fct = function<R(B, Ps...)>;

    binder1st(fct const& f, B const b) : _f(f), _b(b) {}

    R operator()(Ps const... params) const { return _f(_b, params...); }

private:
    fct _f;
    B _b;
};

template<typename R, typename... Ps>
closure<R(Ps...)> bind_all(function<R(Ps...)> const& f, Ps const... params)
{
    return closure<R(Ps...)>(f, params...);
}

template<typename R, typename B, typename... Ps>
binder1st<R(B, Ps...)> bind1st(function<R(B, Ps...)> const& f, B const& b)
{
    return binder1st<R(B, Ps...)>(f, b);
}

template<typename R, typename... Args>
struct MakeFunction
{
    MakeFunction() = delete;

    using f = function<R(Args...)>;

    template<typename T>
    static f make(T& t)
    {
        static_assert(
            !std::is_const<T>::value,
            "functional object can't be const due to potential lifetime issues");
        return f::template create<T, &T::operator()>(t);
    }
};

template<typename R, typename... Args>
struct MakeFunction<R(Args...)> : MakeFunction<R, Args...>
{};

template<typename R, typename Cls, typename... Args>
struct MakeFunction<R (Cls::*)(Args...) const> : MakeFunction<R, Args...>
{};

template<typename R, typename Cls, typename... Args>
struct MakeFunction<R (Cls::*)(Args...)> : MakeFunction<R, Args...>
{};

template<typename F>
auto make_function(F& f) -> typename MakeFunction<decltype(&F::operator())>::f
{
    return MakeFunction<decltype(&F::operator())>::make(f);
}

template<typename F>
auto make_function(F* f) -> typename MakeFunction<F>::f
{
    return MakeFunction<F>::make(f);
}

} // namespace estd

#endif // GUARD_05AF1AC2_4439_47BF_8B23_BF54CFC4327C
