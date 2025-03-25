// Copyright 2024 Accenture.

/**
 * Contains estd::ratio class.
 * \file
 * \ingroup estl_time
 */
#pragma once

#include <platform/estdint.h>

#include <type_traits>

#ifndef ESTD_NO_CHRONO

namespace estd
{
/// \cond INTERNAL
namespace internal
{
template<intmax_t T>
struct _static_sign : std::integral_constant<intmax_t, ((T < 0) ? -1 : 1)>
{};

template<intmax_t T>
struct _static_abs : std::integral_constant<uintmax_t, T * _static_sign<T>::value>
{};

template<intmax_t P, intmax_t Q>
struct _static_gcd : _static_gcd<Q, (P % Q)>
{};

template<intmax_t P>
struct _static_gcd<P, 0> : std::integral_constant<intmax_t, _static_abs<P>::value>
{};

template<intmax_t Q>
struct _static_gcd<0, Q> : std::integral_constant<intmax_t, _static_abs<Q>::value>
{};

template<uintmax_t X>
struct _static_csb : std::integral_constant<uintmax_t, _static_csb<(X >> 1U)>::value + 1U>
{};

template<>
struct _static_csb<0> : std::integral_constant<uintmax_t, 0U>
{};

template<uintmax_t X>
struct _static_clz
: std::integral_constant<uintmax_t, _static_csb<UINTMAX_MAX>::value - _static_csb<X>::value>
{};

template<intmax_t X, intmax_t Y>
struct _safe_multiply
{
private:
    static constexpr uintmax_t _c
        = static_cast<uintmax_t>(1) << static_cast<uintmax_t>((sizeof(intmax_t) * 4U));

    static constexpr uintmax_t _a0 = _static_abs<X>::value % _c;
    static constexpr uintmax_t _a1 = _static_abs<X>::value / _c;
    static constexpr uintmax_t _b0 = _static_abs<Y>::value % _c;
    static constexpr uintmax_t _b1 = _static_abs<Y>::value / _c;

public:
    static constexpr intmax_t value = X * Y;
};

template<uintmax_t Hi1, uintmax_t Lo1, uintmax_t Hi2, uintmax_t Lo2>
struct _big_less : std::integral_constant<bool, ((Hi1 < Hi2) || ((Hi1 == Hi2) && (Lo1 < Lo2)))>
{};

template<uintmax_t Hi1, uintmax_t Lo1, uintmax_t Hi2, uintmax_t Lo2>
struct _big_add
{
    static constexpr uintmax_t _lo = Lo1 + Lo2;
    static constexpr uintmax_t _carry
        = ((Lo1 + Lo2) < Lo1) ? static_cast<uintmax_t>(1UL) : static_cast<uintmax_t>(0UL);
    static constexpr uintmax_t _hi = static_cast<uintmax_t>(Hi1 + Hi2 + _carry);
};

template<uintmax_t Hi1, uintmax_t Lo1, uintmax_t Hi2, uintmax_t Lo2>
struct _big_sub
{
    static constexpr uintmax_t _lo    = Lo1 - Lo2;
    static constexpr uintmax_t _carry = static_cast<uintmax_t>(
        (Lo1 < Lo2) ? static_cast<uintmax_t>(1U) : static_cast<uintmax_t>(0U));
    static constexpr uintmax_t _hi = static_cast<uintmax_t>((Hi1 - Hi2) - _carry);
};

template<uintmax_t X, uintmax_t Y>
struct _big_mul
{
private:
    static constexpr uintmax_t _c
        = static_cast<uintmax_t>(1) << static_cast<uintmax_t>(sizeof(intmax_t) * 4U);
    static constexpr uintmax_t _x0     = X % _c;
    static constexpr uintmax_t _x1     = X / _c;
    static constexpr uintmax_t _y0     = Y % _c;
    static constexpr uintmax_t _y1     = Y / _c;
    static constexpr uintmax_t _x0y0   = static_cast<uintmax_t>(_x0 * _y0);
    static constexpr uintmax_t _x0y1   = static_cast<uintmax_t>(_x0 * _y1);
    static constexpr uintmax_t _x1y0   = static_cast<uintmax_t>(_x1 * _y0);
    static constexpr uintmax_t _x1y1   = static_cast<uintmax_t>(_x1 * _y1);
    static constexpr uintmax_t _mix    = static_cast<uintmax_t>(_x0y1 + _x1y0);
    static constexpr uintmax_t _mix_lo = _mix * _c;
    static constexpr uintmax_t _mix_hi = (_mix / _c) + ((_mix < _x0y1) ? _c : 0U);
    using res_t                        = _big_add<_mix_hi, _mix_lo, _x1y1, _x0y0>;

public:
    static constexpr uintmax_t _hi = res_t::_hi;
    static constexpr uintmax_t _lo = res_t::_lo;
};

template<uintmax_t N1, uintmax_t N0, uintmax_t D>
struct _big_div_impl
{
private:
    static constexpr uintmax_t _c
        = static_cast<uintmax_t>(1U) << static_cast<uintmax_t>(sizeof(intmax_t) * 4U);
    static constexpr uintmax_t _d1 = D / _c;
    static constexpr uintmax_t _d0 = D % _c;

    static constexpr uintmax_t _q1x = static_cast<uintmax_t>(N1 / _d1);
    static constexpr uintmax_t _r1x = static_cast<uintmax_t>(N1 % _d1);
    static constexpr uintmax_t _m   = static_cast<uintmax_t>(_q1x * _d0);
    static constexpr uintmax_t _r1y = (_r1x * _c) + (N0 / _c);
    static constexpr uintmax_t _r1z = _r1y + D;
    static constexpr uintmax_t _r1  = static_cast<uintmax_t>(
        ((_r1y < _m) ? ((_r1z >= D) && (_r1z < _m)) ? (_r1z + D) : _r1z : _r1y) - _m);
    static constexpr uintmax_t _q1 = static_cast<uintmax_t>(
        _q1x - ((_r1y < _m) ? ((_r1z >= D) && (_r1z < _m)) ? static_cast<uintmax_t>(2U) : 1U : 0U));
    static constexpr uintmax_t _q0x = static_cast<uintmax_t>(_r1 / _d1);
    static constexpr uintmax_t _r0x = static_cast<uintmax_t>(_r1 % _d1);
    static constexpr uintmax_t _n   = static_cast<uintmax_t>(_q0x * _d0);
    static constexpr uintmax_t _r0y = (_r0x * _c) + (N0 % _c);
    static constexpr uintmax_t _r0z = _r0y + D;
    static constexpr uintmax_t _r0  = static_cast<uintmax_t>(
        ((_r0y < _n) ? ((_r0z >= D) && (_r0z < _n)) ? (_r0z + D) : _r0z : _r0y) - _n);
    static constexpr uintmax_t _q0 = static_cast<uintmax_t>(
        _q0x - ((_r0y < _n) ? (((_r0z >= D) && (_r0z < _n)) ? 2U : 1U) : 0U));

public:
    static constexpr uintmax_t _quot = static_cast<uintmax_t>((_q1 * _c) + _q0);
    static constexpr uintmax_t _rem  = _r0;

private:
    using prod_t = _big_mul<_quot, D>;
    using sum_t  = _big_add<prod_t::_hi, prod_t::_lo, 0, _rem>;
};

template<uintmax_t N1, uintmax_t N0, uintmax_t D>
struct _big_div
{
private:
    static constexpr uintmax_t _shift = _static_clz<D>::value;
    static constexpr uintmax_t _coshift_
        = static_cast<uintmax_t>((sizeof(uintmax_t) * static_cast<uintmax_t>(8U)) - _shift);
    static constexpr uintmax_t _coshift = static_cast<uintmax_t>((_shift != 0U) ? _coshift_ : 0U);
    static constexpr uintmax_t _c1 = static_cast<uintmax_t>(static_cast<uintmax_t>(1U) << _shift);
    static constexpr uintmax_t _c2 = static_cast<uintmax_t>(static_cast<uintmax_t>(1U) << _coshift);
    static constexpr uintmax_t _new_d      = static_cast<uintmax_t>(D * _c1);
    static constexpr uintmax_t _new_n0     = static_cast<uintmax_t>(N0 * _c1);
    static constexpr uintmax_t _n1_shifted = static_cast<uintmax_t>((N1 % D) * _c1);
    static constexpr uintmax_t _n0_top = static_cast<uintmax_t>((_shift != 0U) ? (N0 / _c2) : 0U);
    static constexpr uintmax_t _new_n1 = static_cast<uintmax_t>(_n1_shifted + _n0_top);
    using res_t                        = _big_div_impl<_new_n1, _new_n0, _new_d>;

public:
    static constexpr uintmax_t _quot_hi = N1 / D;
    static constexpr uintmax_t _quot_lo = res_t::_quot;
    static constexpr uintmax_t _rem     = res_t::_rem / _c1;

private:
    using p0_t  = _big_mul<_quot_lo, D>;
    using p1_t  = _big_mul<_quot_hi, D>;
    using sum_t = _big_add<p0_t::_hi, p0_t::_lo, p1_t::_lo, _rem>;
};

} // namespace internal

/// \endcond

/**
 * This template is used to instantiate types that represent a finite rational number denoted by a
 * numerator and a denominator. The numerator and denominator are implemented as compile-time
 * constants of type intmax_t. Notice that the ratio is not represented by an object of this type,
 * but by the type itself, which uses compile-time constant members to define the ratio. Therefore,
 * ratio can only be used to express constants and cannot contain any value.
 *
 * Types of this template are used on the standard class duration (see header chrono).
 *
 * \tparam N Numerator. Its absolute value shall be in the range of representable values of
 * intmax_t. intmax_t is the widest signed integer type. \tparam D Denominator. Its absolute value
 * shall be in the range of representable values of intmax_t, and shall not be zero. intmax_t is the
 * widest signed integer type.
 *
 * The values of the static constants num and den represent the unique lowest reduction of the ratio
 * N:D. This means that, in some cases, num and den are not the same as the template arguments N
 * and D: if the greatest common divisor among N and D is not one, num and den are the results of
 * dividing N and D by that greatest common divisor. The sign is always represented by num (den is
 * always positive): if D is negative, the sign of num is the opposite of that of N.
 */
template<intmax_t N, intmax_t D = 1>
class ratio
{
public:
    /**
     */
    static constexpr intmax_t num
        = (N * internal::_static_sign<D>::value) / internal::_static_gcd<D, N>::value;
    static constexpr intmax_t den = static_cast<intmax_t>(internal::_static_abs<D>::value)
                                    / internal::_static_gcd<D, N>::value;

    using type = ratio<num, den>;
};

template<intmax_t N, intmax_t D>
constexpr intmax_t ratio<N, D>::den;
template<intmax_t N, intmax_t D>
constexpr intmax_t ratio<N, D>::num;

/// \cond INTERNAL
namespace internal
{
template<
    typename R1,
    typename R2,
    typename Left  = _big_mul<R1::num, R2::den>,
    typename Right = _big_mul<R2::num, R1::den>>
struct _ratio_less_impl
: std::integral_constant<bool, _big_less<Left::_hi, Left::_lo, Right::_hi, Right::_lo>::value>
{};

template<
    typename R1,
    typename R2,
    bool
    = ((R1::num == 0) || (R2::num == 0)
       || (_static_sign<R1::num>::value != _static_sign<R2::num>::value)),
    bool = ((_static_sign<R1::num>::value < 0) && (_static_sign<R2::num>::value < 0))>
struct _ratio_less : _ratio_less_impl<R1, R2>::type
{};

template<typename R1, typename R2>
struct _ratio_less<R1, R2, true, false> : std::integral_constant<bool, (R1::num < R2::num)>
{};

template<typename R1, typename R2>
struct _ratio_less<R1, R2, false, true>
: _ratio_less_impl<ratio<-R2::num, R2::den>, ratio<-R1::num, R1::den>>::type
{};
} // namespace internal

/// \endcond

/**
 * If the ratios R1 and R2 are equal, provides the member constant value equal true. Otherwise,
 * value is false.
 */
template<typename R1, typename R2>
struct ratio_equal : std::integral_constant<bool, (R1::num == R2::num) && (R1::den == R2::den)>
{};

/**
 * If the ratios R1 and R2 are not equal, provides the member constant value equal true. Otherwise,
 * value is false.
 */
template<typename R1, typename R2>
struct ratio_not_equal : std::integral_constant<bool, ratio_equal<R1, R2>::value == false>
{};

/**
 * If the ratio R1 is less than the ratio R2, provides the member constant value equal true.
 * Otherwise, value is false.
 */
template<typename R1, typename R2>
struct ratio_less : internal::_ratio_less<R1, R2>::type
{};

/**
 * If the ratio R1 is less than or equal to the ratio R2, provides the member constant value equal
 * true. Otherwise, value is false.
 */
template<typename R1, typename R2>
struct ratio_less_equal : std::integral_constant<bool, ratio_less<R2, R1>::value == false>
{};

/**
 * If the ratio R1 is greater than the ratio R2, provides the member constant value equal true.
 * Otherwise, value is false.
 */
template<typename R1, typename R2>
struct ratio_greater : std::integral_constant<bool, ratio_less<R2, R1>::value>
{};

/**
 * If the ratio R1 is greater than or equal to the ratio R2, provides the member constant value
 * equal true. Otherwise, value is false.
 */
template<typename R1, typename R2>
struct ratio_greater_equal : std::integral_constant<bool, ratio_less<R1, R2>::value == false>
{};

/// \cond INTERNAL
namespace internal
{
template<
    typename R1,
    typename R2,
    bool = (R1::num >= 0),
    bool = (R2::num >= 0),
    bool = ratio_less<
        ratio<_static_abs<R1::num>::value, R1::den>,
        ratio<_static_abs<R2::num>::value, R2::den>>::value>
struct _ratio_add_impl
{
private:
    using sum_t =
        typename _ratio_add_impl<ratio<-R1::num, R1::den>, ratio<-R2::num, R2::den>>::type;

public:
    using type = ratio<-sum_t::num, sum_t::den>;
};

template<typename R1, typename R2, bool B>
struct _ratio_add_impl<R1, R2, true, true, B>
{
private:
    static constexpr uintmax_t _g  = static_cast<uintmax_t>(_static_gcd<R1::den, R2::den>::value);
    static constexpr uintmax_t _d2 = static_cast<uintmax_t>(R2::den / _g);
    using d_t                      = _big_mul<R1::den, _d2>;
    using x_t                      = _big_mul<R1::num, R2::den / _g>;
    using y_t                      = _big_mul<R2::num, R1::den / _g>;
    using n_t                      = _big_add<x_t::_hi, x_t::_lo, y_t::_hi, y_t::_lo>;
    using ng_t                     = _big_div<n_t::_hi, n_t::_lo, _g>;
    static constexpr uintmax_t _g2 = static_cast<uintmax_t>(_static_gcd<ng_t::_rem, _g>::value);
    using n_final_t                = _big_div<n_t::_hi, n_t::_lo, _g2>;
    using d_final_t                = _big_mul<R1::den / _g2, _d2>;

public:
    using type = ratio<n_final_t::_quot_lo, d_final_t::_lo>;
};

template<typename R1, typename R2>
struct _ratio_add_impl<R1, R2, false, true, true> : _ratio_add_impl<R2, R1>
{};

template<typename R1, typename R2>
struct _ratio_add_impl<R1, R2, true, false, false>
{
private:
    static constexpr uintmax_t _g  = static_cast<uintmax_t>(_static_gcd<R1::den, R2::den>::value);
    static constexpr uintmax_t _d2 = static_cast<uintmax_t>(R2::den / _g);
    using d_t                      = _big_mul<R1::den, _d2>;
    using x_t                      = _big_mul<R1::num, R2::den / _g>;
    using y_t                      = _big_mul<-R2::num, R1::den / _g>;
    using n_t                      = _big_sub<x_t::_hi, x_t::_lo, y_t::_hi, y_t::_lo>;
    using ng_t                     = _big_div<n_t::_hi, n_t::_lo, _g>;
    static constexpr uintmax_t _g2 = static_cast<uintmax_t>(_static_gcd<ng_t::_rem, _g>::value);
    using n_final_t                = _big_div<n_t::_hi, n_t::_lo, _g2>;
    using d_final_t                = _big_mul<R1::den / _g2, _d2>;

public:
    using type = ratio<n_final_t::_quot_lo, d_final_t::_lo>;
};

template<typename R1, typename R2>
struct _ratio_add
{
    using type                    = typename _ratio_add_impl<R1, R2>::type;
    static constexpr intmax_t num = type::num;
    static constexpr intmax_t den = type::den;
};

template<typename R1, typename R2>
constexpr intmax_t _ratio_add<R1, R2>::num;
template<typename R1, typename R2>
constexpr intmax_t _ratio_add<R1, R2>::den;

template<typename R1, typename R2>
struct _ratio_subtract
{
    using type                    = typename _ratio_add<R1, ratio<-R2::num, R2::den>>::type;
    static constexpr intmax_t num = type::num;
    static constexpr intmax_t den = type::den;
};

template<typename R1, typename R2>
constexpr intmax_t _ratio_subtract<R1, R2>::num;
template<typename R1, typename R2>
constexpr intmax_t _ratio_subtract<R1, R2>::den;

template<typename R1, typename R2>
struct _ratio_multiply
{
private:
    static constexpr intmax_t _gcd1 = _static_gcd<R1::num, R2::den>::value;
    static constexpr intmax_t _gcd2 = _static_gcd<R2::num, R1::den>::value;

public:
    using type = ratio<
        _safe_multiply<(R1::num / _gcd1), (R2::num / _gcd2)>::value,
        _safe_multiply<(R1::den / _gcd2), (R2::den / _gcd1)>::value>;
    static constexpr intmax_t num = type::num;
    static constexpr intmax_t den = type::den;
};

template<typename R1, typename R2>
constexpr intmax_t _ratio_multiply<R1, R2>::num;
template<typename R1, typename R2>
constexpr intmax_t _ratio_multiply<R1, R2>::den;

template<typename R1, typename R2>
struct _ratio_divide
{
    using type                    = typename _ratio_multiply<R1, ratio<R2::den, R2::num>>::type;
    static constexpr intmax_t num = type::num;
    static constexpr intmax_t den = type::den;
};

template<typename R1, typename R2>
constexpr intmax_t _ratio_divide<R1, R2>::num;
template<typename R1, typename R2>
constexpr intmax_t _ratio_divide<R1, R2>::den;

} // namespace internal

/// \endcond

/**
 * The alias template estd::ratio_add denotes the result of adding two exact rational
 * fractions represented by the estd::ratio specializations R1 and R2.
 *
 * \note
 * If U or V is not representable in intmax_t, the program is ill-formed. If Num or Denom is not
 * representable in intmax_t, the program is ill-formed unless the implementation yields correct
 * values for U and V. The above definition requires that the result of estd::ratio_add<R1,
 * R2> be already reduced to lowest terms; for example,
 * estd::ratio_add<estd::ratio<1, 3>, estd::ratio<1, 6> > is the same
 * type as estd::ratio<1, 2>.
 */
template<typename R1, typename R2>
struct ratio_add : internal::_ratio_add<R1, R2>
{};

/**
 * The alias template estd::ratio_subtract denotes the result of subtracting two exact
 * rational fractions represented by the estd::ratio specializations R1 and R2.
 *
 * \note
 * If U or V is not representable in intmax_t, the program is ill-formed. If Num or Denom is not
 * representable in intmax_t, the program is ill-formed unless the implementation yields correct
 * values for U and V. The above definition requires that the result of
 * estd::ratio_subtract<R1, R2> be already reduced to lowest terms; for example,
 * estd::ratio_subtract<std::ratio<1, 2>, estd::ratio<1, 6> > is the same type as
 * estd::ratio<1, 3>.
 */
template<typename R1, typename R2>
struct ratio_subtract : internal::_ratio_subtract<R1, R2>
{};

/**
 * The alias template estd::ratio_multiply denotes the result of multiplying two exact
 * rational fractions represented by the estd::ratio specializations R1 and R2.
 *
 * \note
 * If U or V is not representable in intmax_t, the program is ill-formed. If Num or Denom is not
 * representable in intmax_t, the program is ill-formed unless the implementation yields correct
 * values for U and V. The above definition requires that the result of
 * estd::ratio_multiply<R1, R2> be already reduced to lowest terms; for example,
 * estd::ratio_multiply<std::ratio<1, 6>, estd::ratio<4, 5> > is the same type as
 * estd::ratio<2, 15>.
 */
template<typename R1, typename R2>
struct ratio_multiply : internal::_ratio_multiply<R1, R2>
{};

/**
 * The alias template estd::ratio_divide denotes the result of dividing two exact rational
 * fractions represented by the estd::ratio specializations R1 and R2.
 *
 * \note
 * If U or V is not representable in intmax_t, the program is ill-formed. If Num or Denom is not
 * representable in intmax_t, the program is ill-formed unless the implementation yields correct
 * values for U and V. The above definition requires that the result of
 * estd::ratio_divide<R1, R2> be already reduced to lowest terms; for example,
 * estd::ratio_divide<estd::ratio<1, 12>, estd::ratio<1, 6> > is the same
 * type as estd::ratio<1, 2>.
 */
template<typename R1, typename R2>
struct ratio_divide : internal::_ratio_divide<R1, R2>
{};

using atto  = ratio<1, 1000000000000000000LL>;
using femto = ratio<1, 1000000000000000LL>;
using pico  = ratio<1, 1000000000000LL>;
using nano  = ratio<1, 1000000000LL>;
using micro = ratio<1, 1000000LL>;
using milli = ratio<1, 1000LL>;
using centi = ratio<1, 100LL>;
using deci  = ratio<1, 10LL>;
using deca  = ratio<10LL, 1>;
using hecto = ratio<100LL, 1>;
using kilo  = ratio<1000LL, 1>;
using mega  = ratio<1000000LL, 1>;
using giga  = ratio<1000000000LL, 1>;
using tera  = ratio<1000000000000LL, 1>;
using peta  = ratio<1000000000000000LL, 1>;
using exa   = ratio<1000000000000000000LL, 1>;

} // namespace estd

#endif // ifndef ESTD_NO_CHRONO

