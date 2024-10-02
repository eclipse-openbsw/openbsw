// Copyright 2024 Accenture.

/**
 * Contains internal type traits
 * \file
 * \ingroup estl_utils
 */
#ifndef GUARD_06190AD2_9E10_4BD2_8764_D2443693629D
#define GUARD_06190AD2_9E10_4BD2_8764_D2443693629D

#include "estd/type_utils.h"

#include <platform/estdint.h>

#include <type_traits> // IWYU pragma: export

namespace estd
{
/// \cond INTERNAL
template<bool C, class T = void>
struct enable_if
{};

template<class T>
struct enable_if<true, T>
{
    using type = T;
};

template<bool C, class T, class F>
struct conditional
{
    using type = T;
};

template<class T, class F>
struct conditional<false, T, F>
{
    using type = F;
};

namespace internal
{
using _true = int8_t;

struct _false
{
    int8_t _[2];
};
} // namespace internal

template<class T, T v>
struct integral_constant
{
    static T const value = v;
    using value_type     = T;
    using type           = integral_constant;
};

using true_type  = integral_constant<bool, true>;
using false_type = integral_constant<bool, false>;

// Used for non-const T
template<class B, class T>
inline uint8_t* after_me(T* const p)
{
    struct X
    {
        T t;
        B b;
    };

    return reinterpret_cast<uint8_t*>(&reinterpret_cast<X*>(p)->b);
}

// used for const T
template<class B, class T>
inline uint8_t const* after_me(T const* const p)
{
    struct X
    {
        T t;
        B b;
    };

    return reinterpret_cast<uint8_t const*>(&reinterpret_cast<X const*>(p)->b);
}

template<template<class T2> class Trait, class T>
bool value_trait(T&)
{
    return Trait<T>::value;
}

template<class T>
struct add_reference
{
    using type = T&;
};

template<class T>
struct add_reference<T&>
{
    using type = T&;
};

namespace internal
{
template<class T>
struct _is_reference : public false_type
{};

template<class T>
struct _is_reference<T&> : public true_type
{};
} // namespace internal

template<class T>
struct is_reference : internal::_is_reference<typename std::remove_cv<T>::type>
{};

template<typename T>
using is_void = std::is_void<T>;

template<typename T>
using is_const = std::is_const<T>;

template<typename T>
using is_array = std::is_array<T>;

template<typename T>
using is_pointer = std::is_pointer<T>;

template<typename T>
using is_class = std::is_class<T>;

template<typename T, typename U>
using is_same = std::is_same<T, U>;

template<class B, class D>
using is_base_of = std::is_base_of<B, D>;

template<class T>
using is_standard_layout = std::is_standard_layout<T>;

namespace internal
{
template<bool isObject, typename T>
struct is_callable_impl
: public std::is_function<
      typename std::remove_pointer<typename std::remove_reference<T>::type>::type>::type
{};

template<typename T>
struct is_callable_impl<true, T>
{
private:
    struct Fallback
    {
        void operator()();
    };

    struct Derived
    : T
    , Fallback
    {};

    template<typename U, U>
    struct Check;

    template<typename>
    static std::true_type test(...);

    template<typename C>
    static std::false_type test(Check<void (Fallback::*)(), &C::operator()>*);

public:
    using type = decltype(test<Derived>(nullptr));
};
} // namespace internal

template<typename T>
using is_callable = typename internal::is_callable_impl<
    std::is_class<typename std::remove_reference<T>::type>::value,
    typename std::remove_reference<T>::type>::type;

namespace internal
{
template<class T>
struct _is_member_function_pointer : public false_type
{};

template<class T, class F>
struct _is_member_function_pointer<T F::*> : public std::is_function<T>
{};
} // namespace internal

template<class T>
struct is_member_function_pointer
: public internal::_is_member_function_pointer<typename std::remove_cv<T>::type>
{};

namespace internal
{
template<class T, bool Integral, bool Signed, bool Unsigned, class SignedType, class UnsignedType>
struct _type_traits_base_impl
{
    static bool const is_integral = Integral;
    static bool const is_signed   = Signed;
    static bool const is_unsigned = Unsigned;
    using signedType              = SignedType;
    using unsignedType            = UnsignedType;
};

template<class T, int Ordinal, class UnsignedType>
struct _signed_integral_type_traits_impl
: _type_traits_base_impl<T, true, true, false, T, UnsignedType>
{
    static int const ordinal = Ordinal;
};

template<class T, int Ordinal, class SignedType>
struct _unsigned_integral_type_traits_impl
: _type_traits_base_impl<T, true, false, true, SignedType, T>
{
    static int const ordinal = Ordinal;
};

template<class T>
struct _type_traits_impl : _type_traits_base_impl<T, false, false, false, T, T>
{};

template<>
struct _type_traits_impl<bool> : _signed_integral_type_traits_impl<bool, 0, bool>
{};

template<>
struct _type_traits_impl<signed char>
: _signed_integral_type_traits_impl<signed char, 1, unsigned char>
{};

template<>
struct _type_traits_impl<unsigned char>
: _unsigned_integral_type_traits_impl<unsigned char, 1, signed char>
{};

template<>
struct _type_traits_impl<signed short>
: _signed_integral_type_traits_impl<signed short, 2, unsigned short>
{};

template<>
struct _type_traits_impl<unsigned short>
: _unsigned_integral_type_traits_impl<unsigned short, 2, signed short>
{};

template<>
struct _type_traits_impl<signed int>
: _signed_integral_type_traits_impl<signed int, 3, unsigned int>
{};

template<>
struct _type_traits_impl<unsigned int>
: _unsigned_integral_type_traits_impl<unsigned int, 3, signed int>
{};

template<>
struct _type_traits_impl<signed long>
: _signed_integral_type_traits_impl<signed long, 4, unsigned long>
{};

template<>
struct _type_traits_impl<unsigned long>
: _unsigned_integral_type_traits_impl<unsigned long, 4, signed long>
{};

template<>
struct _type_traits_impl<signed long long>
: _signed_integral_type_traits_impl<signed long long, 5, unsigned long long>
{};

template<>
struct _type_traits_impl<unsigned long long>
: _unsigned_integral_type_traits_impl<unsigned long long, 5, signed long long>
{};
} // namespace internal

template<class T>
struct make_signed
{
    using type = typename internal::_type_traits_impl<T>::signedType;
};

template<class T>
struct make_unsigned
{
    using type = typename internal::_type_traits_impl<T>::unsignedType;
};

namespace internal
{
template<class T>
struct _integral_ordinal : integral_constant<int, internal::_type_traits_impl<T>::ordinal>
{};

template<
    class T1,
    class T2,
    bool = ((std::is_integral<T1>::value == true) && (std::is_integral<T2>::value == true)),
    bool = (_integral_ordinal<T1>::value > _integral_ordinal<T2>::value),
    bool = (_integral_ordinal<T1>::value < _integral_ordinal<T2>::value),
    bool = (std::is_signed<T1>::value)>
struct _common_integral_type
{};

template<class T1, class T2, bool S>
struct _common_integral_type<T1, T2, true, true, false, S>
{
    using type = T1;
};

template<class T1, class T2, bool S>
struct _common_integral_type<T1, T2, true, false, true, S>
{
    using type = T2;
};

template<class T1, class T2>
struct _common_integral_type<T1, T2, true, false, false, false>
{
    using type = T1;
};

template<class T1, class T2>
struct _common_integral_type<T1, T2, true, false, false, true>
{
    using type = T2;
};

template<
    class T1,
    class T2,
    bool = ((std::is_integral<T1>::value == true) && (std::is_integral<T2>::value == true))>
struct _common_type_impl
{};

template<class T>
struct _common_type_impl<T, T, false> : true_type
{};

template<class T1, class T2>
struct _common_type_impl<T1, T2, true> : _common_integral_type<T1, T2>
{};
} // namespace internal

template<class T1, class T2>
struct common_type : internal::_common_type_impl<T1, T2>
{};

/// \endcond

template<typename T>
struct size_of
{
    enum V
    {
        value = sizeof(T)
    };
};

template<>
struct size_of<void>
{
    enum
    {
        value = 0
    };
};

template<typename T>
struct align_of
{
    enum V
    {
        value = __alignof__(T)
    };
};

template<>
struct align_of<void>
{
    enum
    {
        value = 1
    };
};

template<typename T, typename A = T>
struct alignas(A) aligned_mem
{
    uint8_t data[sizeof(T)];

    T* cast_to_type() { return type_utils<T>::cast_to_type(&data[0]); }

    T const* cast_const_to_type() const { return type_utils<T>::cast_const_to_type(&data[0]); }
};

} // namespace estd

#endif // GUARD_06190AD2_9E10_4BD2_8764_D2443693629D
