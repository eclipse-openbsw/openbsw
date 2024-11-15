// chrono.h port from OpenBSW
//
// Copyright 2024 Accenture.
//
// License: Apache 2.0

/**
 * Contains etl::chrono namespace.
 * \file
 * \ingroup etl_time
 */
#ifndef ETL_CHRONO_INCLUDED
#define ETL_CHRONO_INCLUDED

#include <type_traits>

#include "platform.h"

#include "limits.h"
#include "ratio.h"
#include "type_traits.h"

#ifndef ETL_NO_CHRONO

/*
 * Functions to implement for full chrono support on your platform.
 */

extern "C" uint64_t getSystemTimeNs();

namespace etl
{
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

namespace chrono
{
/**
 * \file chrono.h
 *
 * C++11 like implementation of the chrono time functionality.
 * chrono is the name of a header, but also of a sub-namespace: All the elements in this header
 * (except for the common_type specializations) are not defined directly under the etl
 * namespace (like most of the ETL) but under the etl::chrono namespace.
 *
 * \section chrono_library chrono library
 *
 * The chrono library defines three main types (durations, clocks, and time points) as well as
 * utility functions and common typedefs.
 *
 * \subsection chrono_library_duration Duration
 *
 * A duration consists of a span of time, defined as some number of ticks of some time unit. For
 * example, "42 seconds" could be represented by a duration consisting of 42 ticks of a 1-second
 * time unit.
 *
 * - etl::chrono::duration
 *   a time interval
 *
 * \subsection chrono_library_clocks Clocks
 *
 * A clock consists of a starting point (or epoch) and a tick rate. The ETL defines a single clock
 * type:
 *
 * - etl::chrono::high_resolution_clock
 *   the clock with the shortest tick period available
 *
 * \subsection chrono_library_clocks Time point
 *
 * A time point is a duration of time that has passed since the epoch of specific clock.
 *
 * - etl::chrono::time_point
 *   A point in time
 */

template<typename Rep, typename Period = ratio<1>>
struct duration;

template<typename Clock, typename Dur = typename Clock::duration>
struct time_point;

/// \cond INTERNAL
namespace internal
{
template<class T>
struct _is_duration : false_type
{};

template<typename Rep, typename Period>
struct _is_duration<duration<Rep, Period>> : true_type
{};

template<class T>
struct _is_ratio : false_type
{};

template<intmax_t R1, intmax_t R2>
struct _is_ratio<ratio<R1, R2>> : true_type
{};

template<
    typename ToDur,
    typename CF,
    typename CR,
    bool NumIsOne = (CF::num == 1),
    bool DenIsOne = (CF::den == 1)>
struct _duration_cast_impl
{
    template<typename Rep, typename Period>
    static ToDur _cast(duration<Rep, Period> const& dur);
};

template<typename ToDur, typename CF, typename CR>
struct _duration_cast_impl<ToDur, CF, CR, true, true>
{
    template<typename Rep, typename Period>
    static ToDur _cast(duration<Rep, Period> const& dur);
};

template<typename ToDur, typename CF, typename CR>
struct _duration_cast_impl<ToDur, CF, CR, true, false>
{
    template<typename Rep, typename Period>
    static ToDur _cast(duration<Rep, Period> const& dur);
};

template<typename ToDur, typename CF, typename CR>
struct _duration_cast_impl<ToDur, CF, CR, false, true>
{
    template<typename Rep, typename Period>
    static ToDur _cast(duration<Rep, Period> const& dur);
};

template<typename Rep1, typename Period1, typename Rep2, typename Period2>
struct _common_duration_type_impl
{
private:
    using gcd_num_t = ::etl::internal::_static_gcd<Period1::num, Period2::num>;
    using gcd_den_t = ::etl::internal::_static_gcd<Period1::den, Period2::den>;
    using cr_t      = typename ::etl::internal::_common_integral_type<Rep1, Rep2>::type;
    using r_t       = ratio<gcd_num_t::value, (Period1::den / gcd_den_t::value) * Period2::den>;

public:
    using type = duration<cr_t, r_t>;
};

} // namespace internal

/// \endcond

} // namespace chrono

template<typename Rep1, typename Period1, typename Rep2, typename Period2>
struct common_type<chrono::duration<Rep1, Period1>, chrono::duration<Rep2, Period2>>
: chrono::internal::_common_duration_type_impl<Rep1, Period1, Rep2, Period2>
{};

template<typename Clock, typename Dur1, typename Dur2>
struct common_type<chrono::time_point<Clock, Dur1>, chrono::time_point<Clock, Dur2>>
{
    using ct_t = typename common_type<Dur1, Dur2>::type;
    using type = chrono::time_point<Clock, ct_t>;
};

namespace chrono
{
/**
 * Converts the value of dur into some other duration type, taking into account differences in their
 * periods. The function does not use implicit conversions. Instead, all count values are internally
 * converted into the widest representation (the common_type for the internal count types) and then
 * casted to the destination type, all conversions being done explicitly with static_cast. If the
 * destination type has less precision, the value is truncated. \param dur A duration object.
 * \return The value of dur converted into an object of type ToDur. ToDur shall be an instantiation
 * of duration.
 */
template<typename ToDur, typename Rep, typename Period>
ToDur duration_cast(duration<Rep, Period> const& d);

/**
 * This is a traits class to provide the limits and zero value of the type used to represent the
 * count in a duration object.
 */
template<typename Rep>
struct duration_values
{
    /**
     * Zero value.
     */
    static Rep const zero = ::etl::numeric_limits<Rep>::zero;
    /**
     * Minimum value.
     */
    static Rep const min  = ::etl::numeric_limits<Rep>::min;
    /**
     * Maximum value.
     */
    static Rep const max  = ::etl::numeric_limits<Rep>::max;
};

template<typename Rep>
Rep const duration_values<Rep>::zero;
template<typename Rep>
Rep const duration_values<Rep>::min;
template<typename Rep>
Rep const duration_values<Rep>::max;

/**
 * Class template etl::chrono::duration represents a time interval.
 * It consists of a count of ticks of type Rep and a tick period, where the tick period is a
 * compile-time rational constant representing the number of seconds from one tick to the next. The
 * only data stored in a duration is a tick count of type Rep. Period is included as part of the
 * duration's type, and is only used when converting between different durations.
 *
 * \tparam Rep an arithmetic type representing the number of ticks
 * \tparam Period a etl::ratio representing the tick period (i.e. the number of seconds
 * per tick)
 */
template<typename Rep, typename Period>
struct duration
{
    using rep    = Rep;
    using period = Period;

    /**
     * Constructs a duration representing a zero time interval.
     */
    duration();
    /**
     * Constructs a duration with a bitwise copy of the tick count
     * \param src duration to copy from
     */
    duration(duration const& src);
    /**
     * Constructs a duration from another tick type which is convertible to Rep.
     * \note if Rep2 is not convertible to Rep this will cause an compile error to
     *  indicate possible loss of precision.
     *
     * \param _rep tick count
     */
    template<typename Rep2>
    explicit duration(Rep2 const& rep);
    /**
     * Constructs a duration from another duration type if this can be done without
     * truncation.
     * \note This will cause an compile error if etl::ratio_divide<Period2, Period>::den
     * != 1 which means that conversion won't lead to truncation.
     *
     * \param dur duration to convert from
     */
    template<typename Rep2, typename Period2>
    duration(duration<Rep2, Period2> const& dur);

    /**
     * Assigns the contents of one duration to another.
     */
    duration& operator=(duration const& src);

    /**
     * Returns the number of ticks.
     * \return The number of ticks for this duration.
     */
    rep count() const;

    /**
     * Unary plus.
     * \return a copy of the duration object
     */
    duration operator+() const;
    /**
     * Unary minus, i.e. negation of the tick count.
     * \return a copy of the duration object with the number of ticks negated
     */
    duration operator-() const;

    /**
     * Increments the number of ticks for this duration.
     * \return a reference to this duration object after modification
     */
    duration& operator++();
    /**
     * Increments number of ticks for this duration.
     * \return a copy of the duration object made before modification
     */
    duration operator++(int);

    /**
     * Decrements the number of ticks for this duration.
     * \return a reference to this duration object after modification
     */
    duration& operator--();
    /**
     * Decrements number of ticks for this duration.
     * \return a copy of the duration object made before modification
     */
    duration operator--(int);

    /**
     * Adds the tick count of another duration to the tick count value.
     * \param other duration to add
     * \return a reference to this duration object after modification
     */
    duration& operator+=(duration const& other);
    /**
     * Subtracts the tick count of another duration from the tick count value.
     * \param dur duration to add
     * \return a reference to this duration object after modification
     */
    duration& operator-=(duration const& other);
    /**
     * Multiplies the tick count value with a constant value.
     * \param r factor to multiply with
     * \return a reference to this duration object after modification
     */
    duration& operator*=(rep const& r);
    /**
     * Divides the tick count value by a constant value.
     * \param r constant to divide with
     * \return a reference to this duration object after modification
     */
    duration& operator/=(rep const& r);
    /**
     * Sets the tick count to the remainder of the division by a constant value.
     * \param r constant to divide by
     * \return a reference to this duration object after modification
     */
    duration& operator%=(rep const& r);
    /**
     * Sets the tick count to the remainder of the division by the tick count of another duration
     * \param other duration to divide by
     * \return a reference to this duration object after modification
     */
    duration& operator%=(duration const& other);

    /**
     * \return a duration value of zero.
     */
    static duration zero();
    /**
     * \return the maximum value of duration.
     */
    static duration min();
    /**
     * \return the maximum value of duration.
     */
    static duration max();

private:
    Rep _r;
};

/**
 * Converts the two durations to their common type and creates a duration whose tick count is the
 * sum of the tick counts after conversion. \param lhs duration on the left-hand side of the
 * operator \param rhs duration on the right-hand side of the operator \returns the common type
 * holding the sum of the tick counts after conversion.
 */
template<typename Rep1, typename Period1, typename Rep2, typename Period2>
typename common_type<duration<Rep1, Period1>, common_type<Rep2, Period2>>::type
operator+(duration<Rep1, Period1> const& lhs, duration<Rep2, Period2> const& rhs);

/**
 * Converts the two durations to their common type and creates a duration whose tick count is the
 * right hand number of ticks * subtracted from the left hand number of ticks after conversion.
 * \param lhs duration on the left-hand side of the operator
 * \param rhs duration on the right-hand side of the operator
 * \returns the common type holding the difference of the tick counts after conversion.
 */
template<typename Rep1, typename Period1, typename Rep2, typename Period2>
typename common_type<duration<Rep1, Period1>, common_type<Rep2, Period2>>::type
operator-(duration<Rep1, Period1> const& lhs, duration<Rep2, Period2> const& rhs);

/**
 * Converts the duration dur to one whose rep is the common type between Rep1 and Rep2, and
 * multiplies the number of ticks after conversion by rep. \param dur duration argument \param rep
 * tick count argument \return result of the multiplication of the tick count with the factor
 */
template<typename Rep1, typename Period, typename Rep2>
duration<typename common_type<Rep1, Rep2>::type, Period>
operator*(duration<Rep1, Period> const& d, Rep2 const& r);
/**
 * Converts the duration dur to one whose rep is the common type between Rep1 and Rep2, and
 * multiplies the number of ticks after conversion by r. \param r tick count argument \param dur
 * duration argument \return result of the multiplication of the tick count with the factor r
 */
template<typename Rep1, typename Rep2, typename Period>
duration<typename common_type<Rep2, Rep1>::type, Period>
operator*(Rep1 const& r, duration<Rep2, Period> const& d);

/**
 * Converts the duration dur to one whose r is the common type between Rep1 and Rep2, and divides
 * the number of ticks after conversion by r.
 * \param dur duration argument
 * \param r tick count argument
 * \return result of the division of the tick count by rep
 */
template<typename Rep1, typename Period, typename Rep2>
duration<typename common_type<Rep1, Rep2>::type, Period>
operator/(duration<Rep1, Period> const& d, Rep2 const& r);
/**
 * Converts the two durations to their common type and divides the tick count of left hand side
 * after conversion by the tick count of right hand side after conversion. \note the return value of
 * this operator is not a duration. \param lhs duration on the left-hand side of the operator \param
 * rhs duration on the right-hand side of the operator \return the result of the division of
 * converted tick counts lhs by rhs
 */
template<typename Rep1, typename Period1, typename Rep2, typename Period2>
typename common_type<Rep1, Rep2>::type
operator/(duration<Rep1, Period1> const& lhs, duration<Rep2, Period2> const& rhs);

/**
 * Converts the duration dur to one whose rep is the common type between Rep1 and Rep2, and creates
 * a duration whose tick count is the remainder of the division of the tick count, after conversion,
 * by r. \param dur duration argument \param r tick count argument \return remainder of the division
 * of the tick count by r
 */
template<typename Rep1, typename Period, typename Rep2>
duration<typename common_type<Rep1, Rep2>::type, Period>
operator%(duration<Rep1, Period> const& d, Rep2 const& r);
/**
 * Converts the two durations to their common type and creates a duration whose tick count is the
 * remainder of the tick counts after conversion.
 * \param lhs duration on the left-hand side of the operator
 * \param rhs duration on the right-hand side of the operator
 * \return the remainder of the division of converted tick counts lhs by rhs
 */
template<typename Rep1, typename Period1, typename Rep2, typename Period2>
typename common_type<duration<Rep1, Period1>, duration<Rep2, Period2>>::type
operator%(duration<Rep1, Period1> const& lhs, duration<Rep2, Period2> const& rhs);

/**
 * Converts the two durations to their common type and checks if the number of ticks of both
 * durations are equal. \param lhs duration on the left-hand side of the operator \param rhs
 * duration on the right-hand side of the operator
 *  - true if both durations are equal.
 *  - false otherwise
 */
template<typename Rep1, typename Period1, typename Rep2, typename Period2>
bool operator==(duration<Rep1, Period1> const& lhs, duration<Rep2, Period2> const& rhs);
/**
 * Converts the two durations to their common type and checks if the number of ticks of durations
 * are not equal. \param lhs duration on the left-hand side of the operator \param rhs duration on
 * the right-hand side of the operator
 *  - true if both durations are not equal.
 *  - false otherwise
 */
template<typename Rep1, typename Period1, typename Rep2, typename Period2>
bool operator!=(duration<Rep1, Period1> const& lhs, duration<Rep2, Period2> const& rhs);
/**
 * Converts the two durations to their common type and checks if the number of ticks of the left
 * hand side duration is less than the number of ticks of the right hand side duration. \param lhs
 * duration on the left-hand side of the operator \param rhs duration on the right-hand side of the
 * operator \return
 *  - true if lhs is less than rhs
 *  - false otherwise
 */
template<typename Rep1, typename Period1, typename Rep2, typename Period2>
bool operator<(duration<Rep1, Period1> const& lhs, duration<Rep2, Period2> const& rhs);
/**
 * Converts the two durations to their common type and checks if the number of ticks of the left
 * hand side duration is less than or equal to the number of ticks of the right hand side duration.
 * \param lhs duration on the left-hand side of the operator
 * \param rhs duration on the right-hand side of the operator
 * \return
 *  - true if lhs is less than or equal to rhs
 *  - false otherwise
 */
template<typename Rep1, typename Period1, typename Rep2, typename Period2>
bool operator<=(duration<Rep1, Period1> const& lhs, duration<Rep2, Period2> const& rhs);
/**
 * Converts the two durations to their common type and checks if the number of ticks of the left
 * hand side duration is greater than the number of ticks of the right hand side duration. \param
 * lhs duration on the left-hand side of the operator \param rhs duration on the right-hand side of
 * the operator \return
 *  - true if lhs is greater than to rhs
 *  - false otherwise
 */
template<typename Rep1, typename Period1, typename Rep2, typename Period2>
bool operator>(duration<Rep1, Period1> const& lhs, duration<Rep2, Period2> const& rhs);
/**
 * Converts the two durations to their common type and checks if the number of ticks of the left
 * hand side duration is greater than or equal to the number of ticks of the right hand side
 * duration. \param lhs duration on the left-hand side of the operator \param rhs duration on the
 * right-hand side of the operator \return
 *  - true if lhs is greater than or equal to rhs
 *  - false otherwise
 */
template<typename Rep1, typename Period1, typename Rep2, typename Period2>
bool operator>=(duration<Rep1, Period1> const& lhs, duration<Rep2, Period2> const& rhs);

using nanoseconds  = duration<int64_t, nano>;
using microseconds = duration<int64_t, micro>;
using milliseconds = duration<int64_t, milli>;
using seconds      = duration<int64_t>;
using minutes      = duration<int32_t, ratio<60>>;
using hours        = duration<int32_t, ratio<3600>>;

/**
 * Class template etl::chrono::time_point represents a point in time. It is implemented as
 * if it stores a value of type Duration indicating the time interval from the start of the Clock's
 * epoch. \tparam clock Clock, the clock on which this time point is measured \tparam duration
 * Duration, a etl::chrono::duration type used to measure the time since epoch \tparam rep
 * Rep, an arithmetic type representing the number of ticks of the duration \tparam period  Period,
 * a etl::ratio type representing the tick period of the duration
 */
template<typename Clock, typename Dur>
struct time_point
{
    using clock    = Clock;
    using duration = Dur;
    using rep      = typename duration::rep;
    using period   = typename duration::period;

    /**
     * Default constructor, creates a time_point with a value of Clock's epoch.
     */
    time_point();
    /**
     * Constructs a time_point at Clock's epoch plus a given duration.
     * \param d duration from the Clock's epoch.
     */
    explicit time_point(duration const& d);

    /**
     * Default copy constructor.
     */
    time_point(time_point const& other);

    /**
     * Constructs a time_point by converting another time_point to duration.
     * \note This will cause a compiler error if Dur2 is not implicitly convertible to Dur.
     *
     * \param src time_point to copy converted duration from
     */
    template<typename Dur2>
    explicit time_point(time_point<Clock, Dur2> const& src);

    /**
     * Default assignment operator.
     */
    time_point& operator=(time_point const& other);

    /**
     * Returns the amount of time from the Clock's epoch.
     * \return The amount of time between this time_point and the clock's epoch
     */
    duration time_since_epoch() const;

    /**
     * Applies an offset to the time point.
     *
     * \param d offset to add
     * \return Reference to the object after modification
     */
    time_point& operator+=(duration const& d);
    /**
     * Applies a negative offset to the time point.
     *
     * \param d offset to subtract
     * \return Reference to the object after modification
     */
    time_point& operator-=(duration const& d);

    /**
     * \return the minimum value of time_point.
     */
    static time_point min();
    /**
     * \return the maximum value of time_point.
     */
    static time_point max();

private:
    duration _d;
};

/**
 * Converts the value of tp into a time_point type with a different duration internal object, taking
 * into account differences in their durations's periods. The function uses duration_cast to convert
 * the internal duration objects. \note the function's first template parameter is not the return
 * type, but its duration component. \param tp A time_point object. \return The value of tp
 * converted into an object of type time_point<Clock, ToDur>. time_point<Clock, ToDur> is a
 * time_point with the same Clock type as tp.
 */
template<typename ToDur, typename Clock, typename Dur>
time_point<Clock, ToDur> time_point_cast(time_point<Clock, Dur> const& tp);
/**
 * Copies the time_point lhs object and adds the duration rhs.
 * \param lhs time_point object
 * \param rhs duration to add
 * \return time_point object that represents the sum of lhs and rhs
 */
template<typename Clock, typename Dur1, typename Rep2, typename Period2>
time_point<Clock, typename common_type<Dur1, duration<Rep2, Period2>>::type>
operator+(time_point<Clock, Dur1> const& lhs, duration<Rep2, Period2> const& rhs);
/**
 * Copies the time_point rhs object and adds the duration lhs.
 * \param lhs duration to add
 * \param rhs time_point object
 * \return time_point object that represents the sum of lhs and rhs
 */
template<typename Rep1, typename Period1, typename Clock, typename Dur2>
time_point<Clock, typename common_type<duration<Rep1, Period1>, Dur2>::type>
operator+(duration<Rep1, Period1> const& lhs, time_point<Clock, Dur2> const& rhs);
/**
 * Copies the time_point lhs object and subtracts the duration rhs.
 * \param lhs time_point object
 * \param rhs duration to add
 * \return time_point object that represents the difference of lhs and rhs
 */
template<typename Clock, typename Dur1, typename Rep2, typename Period2>
time_point<Clock, typename common_type<Dur1, duration<Rep2, Period2>>::type>
operator-(time_point<Clock, Dur1> const& lhs, duration<Rep2, Period2> const& rhs);
/**
 * Returns the amount of time between to given time points.
 * \param lhs time_point on the left-hand side of the operator
 * \param rhs time_point on the right-hand side of the operator
 * \return duration that represents the difference of lhs and rhs
 */
template<typename Clock, typename Dur1, typename Dur2>
typename common_type<Dur1, Dur2>::type
operator-(time_point<Clock, Dur1> const& lhs, time_point<Clock, Dur2> const& rhs);
/**
 * Compares two time_points and returns true if they are identical.
 * \param lhs time_point on the left-hand side of the operator
 * \param rhs time_point on the right-hand side of the operator
 * \return
 * - true if the time_points are identical
 * - false otherwise.
 */
template<typename Clock, typename Dur1, typename Dur2>
bool operator==(time_point<Clock, Dur1> const& lhs, time_point<Clock, Dur2> const& rhs);
/**
 * Compares two time_points and returns true if they are not identical.
 * \param lhs time_point on the left-hand side of the operator
 * \param rhs time_point on the right-hand side of the operator
 * \return
 * - true if the time_points are not identical
 * - false otherwise.
 */
template<typename Clock, typename Dur1, typename Dur2>
bool operator!=(time_point<Clock, Dur1> const& lhs, time_point<Clock, Dur2> const& rhs);
/**
 * Compares two time_points and returns true if lhs is less than rhs.
 * \param lhs time_point on the left-hand side of the operator
 * \param rhs time_point on the right-hand side of the operator
 * \return
 * - true if the lhs is less than rhs
 * - false otherwise.
 */
template<typename Clock, typename Dur1, typename Dur2>
bool operator<(time_point<Clock, Dur1> const& lhs, time_point<Clock, Dur2> const& rhs);
/**
 * Compares two time_points and returns true if lhs is less than or equal to rhs.
 * \param lhs time_point on the left-hand side of the operator
 * \param rhs time_point on the right-hand side of the operator
 * \return
 * - true if the lhs is less than or equal to rhs
 * - false otherwise.
 */
template<typename Clock, typename Dur1, typename Dur2>
bool operator<=(time_point<Clock, Dur1> const& lhs, time_point<Clock, Dur2> const& rhs);
/**
 * Compares two time_points and returns true if lhs is greater than rhs.
 * \param lhs time_point on the left-hand side of the operator
 * \param rhs time_point on the right-hand side of the operator
 * \return
 * - true if the lhs is greater than rhs
 * - false otherwise.
 */
template<typename Clock, typename Dur1, typename Dur2>
bool operator>(time_point<Clock, Dur1> const& lhs, time_point<Clock, Dur2> const& rhs);
/**
 * Compares two time_points and returns true if lhs is greater than or equal to rhs.
 * \param lhs time_point on the left-hand side of the operator
 * \param rhs time_point on the right-hand side of the operator
 * \return
 * - true if the lhs is greater than or equal to rhs
 * - false otherwise.
 */
template<typename Clock, typename Dur1, typename Dur2>
bool operator>=(time_point<Clock, Dur1> const& lhs, time_point<Clock, Dur2> const& rhs);

/**
 * Class etl::chrono::high_resolution_clock represents the clock with the smallest
 * tick period provided by the implementation
 */
class high_resolution_clock
{
public:
    using duration   = nanoseconds;
    using rep        = duration::rep;
    using period     = duration::period;
    using time_point = ::etl::chrono::time_point<high_resolution_clock, duration>;

    static bool const is_steady = true;

    /**
     * Returns the current time_point in the frame of the high_resolution_clock.
     * \return The time_point representing the current time.
     */
    static time_point now()
    {
        return time_point(duration(static_cast<int64_t>(getSystemTimeNs())));
    }
};

/*
 * Implementation
 */

template<typename ToDur, typename Rep, typename Period>
ToDur duration_cast(duration<Rep, Period> const& d)
{
    using todur_t     = typename enable_if<internal::_is_duration<ToDur>::value, ToDur>::type;
    using to_period_t = typename ToDur::period;
    using to_rep_t    = typename ToDur::rep;
    using cf_t        = ::etl::ratio_divide<Period, to_period_t>;
    using cr_t        = typename ::etl::internal::_common_integral_type<
        typename ::etl::internal::_common_integral_type<to_rep_t, Rep>::type,
        intmax_t>::type;
    using dc_t = internal::_duration_cast_impl<todur_t, cf_t, cr_t>;
    return dc_t::_cast(d);
}

template<typename Rep, typename Period>
duration<Rep, Period> duration<Rep, Period>::zero()
{
    return duration<Rep, Period>(duration_values<Rep>::zero);
}

template<typename Rep, typename Period>
duration<Rep, Period> duration<Rep, Period>::min()
{
    return duration<Rep, Period>(duration_values<Rep>::min);
}

template<typename Rep, typename Period>
duration<Rep, Period> duration<Rep, Period>::max()
{
    return duration<Rep, Period>(duration_values<Rep>::max);
}

template<typename Rep, typename Period>
duration<Rep, Period>::duration() : _r(0)
{}

template<typename Rep, typename Period>
duration<Rep, Period>::duration(duration<Rep, Period> const& src) : _r(src._r)
{}

template<typename Rep, typename Period>
template<typename Rep2>
duration<Rep, Period>::duration(Rep2 const& rep)
: _r(static_cast<typename enable_if<
         is_same<Rep, typename ::etl::internal::_common_integral_type<Rep, Rep2>::type>::value,
         Rep2>::type>(rep))
{}

template<typename Rep, typename Period>
template<typename Rep2, typename Period2>
duration<Rep, Period>::duration(duration<Rep2, Period2> const& dur)
: _r(
    duration_cast<
        typename enable_if<(ratio_divide<Period2, Period>::den == 1), duration<Rep, Period>>::type>(
        dur)
        .count())
{}

template<typename Rep, typename Period>
duration<Rep, Period>& duration<Rep, Period>::operator=(duration<Rep, Period> const& src)
{
    _r = src._r;
    return *this;
}

template<typename Rep, typename Period>
typename duration<Rep, Period>::rep duration<Rep, Period>::count() const
{
    return _r;
}

template<typename Rep, typename Period>
duration<Rep, Period> duration<Rep, Period>::operator+() const
{
    return *this;
}

template<typename Rep, typename Period>
duration<Rep, Period> duration<Rep, Period>::operator-() const
{
    return duration(-_r);
}

template<typename Rep, typename Period>
duration<Rep, Period>& duration<Rep, Period>::operator++()
{
    ++_r;
    return *this;
}

template<typename Rep, typename Period>
duration<Rep, Period> duration<Rep, Period>::operator++(int)
{
    Rep const curr = _r;
    _r++;
    return duration(curr);
}

template<typename Rep, typename Period>
duration<Rep, Period>& duration<Rep, Period>::operator--()
{
    --_r;
    return *this;
}

template<typename Rep, typename Period>
duration<Rep, Period> duration<Rep, Period>::operator--(int)
{
    Rep const curr = _r;
    _r--;
    return duration(curr);
}

template<typename Rep, typename Period>
duration<Rep, Period>& duration<Rep, Period>::operator+=(duration const& other)
{
    _r += other.count();
    return *this;
}

template<typename Rep, typename Period>
duration<Rep, Period>& duration<Rep, Period>::operator-=(duration const& other)
{
    _r -= other.count();
    return *this;
}

template<typename Rep, typename Period>
duration<Rep, Period>& duration<Rep, Period>::operator*=(rep const& r)
{
    _r *= r;
    return *this;
}

template<typename Rep, typename Period>
duration<Rep, Period>& duration<Rep, Period>::operator/=(rep const& r)
{
    _r /= r;
    return *this;
}

template<typename Rep, typename Period>
duration<Rep, Period>& duration<Rep, Period>::operator%=(rep const& r)
{
    _r %= r;
    return *this;
}

template<typename Rep, typename Period>
duration<Rep, Period>& duration<Rep, Period>::operator%=(duration const& other)
{
    _r %= other.count();
    return *this;
}

template<typename Rep1, typename Period1, typename Rep2, typename Period2>
typename common_type<duration<Rep1, Period1>, duration<Rep2, Period2>>::type
operator+(duration<Rep1, Period1> const& lhs, duration<Rep2, Period2> const& rhs)
{
    using cd_t = typename internal::_common_duration_type_impl<Rep1, Period1, Rep2, Period2>::type;
    return cd_t(cd_t(lhs).count() + cd_t(rhs).count());
}

template<typename Rep1, typename Period1, typename Rep2, typename Period2>
typename common_type<duration<Rep1, Period1>, duration<Rep2, Period2>>::type
operator-(duration<Rep1, Period1> const& lhs, duration<Rep2, Period2> const& rhs)
{
    using cd_t = typename internal::_common_duration_type_impl<Rep1, Period1, Rep2, Period2>::type;
    return cd_t(cd_t(lhs).count() - cd_t(rhs).count());
}

template<typename Rep1, typename Period, typename Rep2>
duration<typename common_type<Rep1, Rep2>::type, Period>
operator*(duration<Rep1, Period> const& d, Rep2 const& r)
{
    using cd_t = duration<typename common_type<Rep1, Rep2>::type, Period>;
    return cd_t(cd_t(d).count() * r);
}

template<typename Rep1, typename Rep2, typename Period>
duration<typename common_type<Rep2, Rep1>::type, Period>
operator*(Rep1 const& r, duration<Rep2, Period> const& d)
{
    return d * r;
}

template<typename Rep1, typename Period, typename Rep2>
duration<typename common_type<Rep1, Rep2>::type, Period>
operator/(duration<Rep1, Period> const& d, Rep2 const& r)
{
    using cd_t = duration<typename common_type<Rep1, Rep2>::type, Period>;
    return cd_t(cd_t(d).count() / r);
}

template<typename Rep1, typename Period1, typename Rep2, typename Period2>
typename common_type<Rep1, Rep2>::type
operator/(duration<Rep1, Period1> const& lhs, duration<Rep2, Period2> const& rhs)
{
    using dur1_t = duration<Rep1, Period1>;
    using dur2_t = duration<Rep2, Period2>;
    using cd_t   = typename common_type<dur1_t, dur2_t>::type;
    return cd_t(lhs).count() / cd_t(rhs).count();
}

template<typename Rep1, typename Period, typename Rep2>
duration<typename common_type<Rep1, Rep2>::type, Period>
operator%(duration<Rep1, Period> const& d, Rep2 const& r)
{
    using cd_t = duration<typename common_type<Rep1, Rep2>::type, Period>;
    return cd_t(cd_t(d).count() % r);
}

template<typename Rep1, typename Period1, typename Rep2, typename Period2>
typename common_type<duration<Rep1, Period1>, duration<Rep2, Period2>>::type
operator%(duration<Rep1, Period1> const& lhs, duration<Rep2, Period2> const& rhs)
{
    using dur1_t = duration<Rep1, Period1>;
    using dur2_t = duration<Rep2, Period2>;
    using cd_t   = typename common_type<dur1_t, dur2_t>::type;
    return cd_t(cd_t(lhs).count() % cd_t(rhs).count());
}

template<typename Rep1, typename Period1, typename Rep2, typename Period2>
bool operator==(duration<Rep1, Period1> const& lhs, duration<Rep2, Period2> const& rhs)
{
    using cd_t = typename internal::_common_duration_type_impl<Rep1, Period1, Rep2, Period2>::type;
    return cd_t(lhs).count() == cd_t(rhs).count();
}

template<typename Rep1, typename Period1, typename Rep2, typename Period2>
bool operator!=(duration<Rep1, Period1> const& lhs, duration<Rep2, Period2> const& rhs)
{
    return !(lhs == rhs);
}

template<typename Rep1, typename Period1, typename Rep2, typename Period2>
bool operator<(duration<Rep1, Period1> const& lhs, duration<Rep2, Period2> const& rhs)
{
    using cd_t = typename internal::_common_duration_type_impl<Rep1, Period1, Rep2, Period2>::type;
    return cd_t(lhs).count() < cd_t(rhs).count();
}

template<typename Rep1, typename Period1, typename Rep2, typename Period2>
bool operator<=(duration<Rep1, Period1> const& lhs, duration<Rep2, Period2> const& rhs)
{
    return !(rhs < lhs);
}

template<typename Rep1, typename Period1, typename Rep2, typename Period2>
bool operator>(duration<Rep1, Period1> const& lhs, duration<Rep2, Period2> const& rhs)
{
    return rhs < lhs;
}

template<typename Rep1, typename Period1, typename Rep2, typename Period2>
bool operator>=(duration<Rep1, Period1> const& lhs, duration<Rep2, Period2> const& rhs)
{
    return !(lhs < rhs);
}

template<typename ToDur, typename Clock, typename Dur>
time_point<Clock, ToDur> time_point_cast(time_point<Clock, Dur> const& tp)
{
    using time_point_t =
        typename enable_if<internal::_is_duration<ToDur>::value, time_point<Clock, ToDur>>::type;
    return time_point_t(duration_cast<ToDur>(tp.time_since_epoch()));
}

template<typename Clock, typename Dur>
time_point<Clock, Dur> time_point<Clock, Dur>::min()
{
    return time_point(duration::min());
}

template<typename Clock, typename Dur>
time_point<Clock, Dur> time_point<Clock, Dur>::max()
{
    return time_point(duration::max());
}

template<typename Clock, typename Dur>
time_point<Clock, Dur>::time_point() : _d(duration::zero())
{}

template<typename Clock, typename Dur>
time_point<Clock, Dur>::time_point(duration const& d) : _d(d)
{}

template<typename Clock, typename Dur>
time_point<Clock, Dur>::time_point(time_point const& other) : _d(other._d)
{}

// explicit
template<typename Clock, typename Dur>
template<typename Dur2>
time_point<Clock, Dur>::time_point(time_point<Clock, Dur2> const& src) : _d(src.time_since_epoch())
{}

template<typename Clock, typename Dur>
time_point<Clock, Dur>& time_point<Clock, Dur>::operator=(time_point const& other)
{
    _d = other._d;
    return *this;
}

template<typename Clock, typename Dur>
typename time_point<Clock, Dur>::duration time_point<Clock, Dur>::time_since_epoch() const
{
    return _d;
}

template<typename Clock, typename Dur>
time_point<Clock, Dur>& time_point<Clock, Dur>::operator+=(duration const& d)
{
    _d += d;
    return *this;
}

template<typename Clock, typename Dur>
time_point<Clock, Dur>& time_point<Clock, Dur>::operator-=(duration const& d)
{
    _d -= d;
    return *this;
}

template<typename Clock, typename Dur1, typename Rep2, typename Period2>
time_point<Clock, typename common_type<Dur1, duration<Rep2, Period2>>::type>
operator+(time_point<Clock, Dur1> const& lhs, duration<Rep2, Period2> const& rhs)
{
    using dur2_t       = duration<Rep2, Period2>;
    using ct_t         = typename common_type<Dur1, dur2_t>::type;
    using time_point_t = time_point<Clock, ct_t>;
    return time_point_t(lhs.time_since_epoch() + rhs);
}

template<typename Rep1, typename Period1, typename Clock, typename Dur2>
time_point<Clock, typename common_type<duration<Rep1, Period1>, Dur2>::type>
operator+(duration<Rep1, Period1> const& lhs, time_point<Clock, Dur2> const& rhs)
{
    using dur1_t       = duration<Rep1, Period1>;
    using ct_t         = typename common_type<dur1_t, Dur2>::type;
    using time_point_t = time_point<Clock, ct_t>;
    return time_point_t(rhs.time_since_epoch() + lhs);
}

template<typename Clock, typename Dur1, typename Rep2, typename Period2>
time_point<Clock, typename common_type<Dur1, duration<Rep2, Period2>>::type>
operator-(time_point<Clock, Dur1> const& lhs, duration<Rep2, Period2> const& rhs)
{
    using dur2_t       = duration<Rep2, Period2>;
    using ct_t         = typename common_type<Dur1, dur2_t>::type;
    using time_point_t = time_point<Clock, ct_t>;
    return time_point_t(lhs.time_since_epoch() - rhs);
}

template<typename Clock, typename Dur1, typename Dur2>
typename common_type<Dur1, Dur2>::type
operator-(time_point<Clock, Dur1> const& lhs, time_point<Clock, Dur2> const& rhs)
{
    return lhs.time_since_epoch() - rhs.time_since_epoch();
}

template<typename Clock, typename Dur1, typename Dur2>
bool operator==(time_point<Clock, Dur1> const& lhs, time_point<Clock, Dur2> const& rhs)
{
    return lhs.time_since_epoch() == rhs.time_since_epoch();
}

template<typename Clock, typename Dur1, typename Dur2>
bool operator!=(time_point<Clock, Dur1> const& lhs, time_point<Clock, Dur2> const& rhs)
{
    return !(lhs == rhs);
}

template<typename Clock, typename Dur1, typename Dur2>
bool operator<(time_point<Clock, Dur1> const& lhs, time_point<Clock, Dur2> const& rhs)
{
    return lhs.time_since_epoch() < rhs.time_since_epoch();
}

template<typename Clock, typename Dur1, typename Dur2>
bool operator<=(time_point<Clock, Dur1> const& lhs, time_point<Clock, Dur2> const& rhs)
{
    return !(rhs < lhs);
}

template<typename Clock, typename Dur1, typename Dur2>
bool operator>(time_point<Clock, Dur1> const& lhs, time_point<Clock, Dur2> const& rhs)
{
    return rhs < lhs;
}

template<typename Clock, typename Dur1, typename Dur2>
bool operator>=(time_point<Clock, Dur1> const& lhs, time_point<Clock, Dur2> const& rhs)
{
    return !(lhs < rhs);
}

namespace internal
{
template<typename ToDur, typename CF, typename CR, bool NumIsOne, bool DenIsOne>
template<typename Rep, typename Period>
ToDur _duration_cast_impl<ToDur, CF, CR, NumIsOne, DenIsOne>::_cast(
    duration<Rep, Period> const& dur)
{
    using to_rep_t = typename ToDur::rep;
    return ToDur(static_cast<to_rep_t>(
        (static_cast<CR>(dur.count()) * static_cast<CR>(CF::num)) / static_cast<CR>(CF::den)));
}

template<typename ToDur, typename CF, typename CR>
template<typename Rep, typename Period>
ToDur _duration_cast_impl<ToDur, CF, CR, true, true>::_cast(duration<Rep, Period> const& dur)
{
    using to_rep_t = typename ToDur::rep;
    return ToDur(static_cast<to_rep_t>(dur.count()));
}

template<typename ToDur, typename CF, typename CR>
template<typename Rep, typename Period>
ToDur _duration_cast_impl<ToDur, CF, CR, true, false>::_cast(duration<Rep, Period> const& dur)
{
    using to_rep_t = typename ToDur::rep;
    return ToDur(static_cast<to_rep_t>(static_cast<CR>(dur.count()) / static_cast<CR>(CF::den)));
}

template<typename ToDur, typename CF, typename CR>
template<typename Rep, typename Period>
ToDur _duration_cast_impl<ToDur, CF, CR, false, true>::_cast(duration<Rep, Period> const& dur)
{
    using to_rep_t = typename ToDur::rep;
    return ToDur(static_cast<to_rep_t>(static_cast<CR>(dur.count()) * static_cast<CR>(CF::num)));
}

} // namespace internal
} // namespace chrono
} // namespace etl


#endif // ifndef ETL_NO_CHRONO

#endif // ETL_CHRONO_INCLUDED
