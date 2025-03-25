// Copyright 2024 Accenture.

/**
 * Contains estd::multi_slice
 * \file
 * \ingroup estl_containers
 */
#pragma once

#include "estd/array.h"
#include "estd/slice.h"

#include <platform/estdint.h>

#include <type_traits>

namespace estd
{
namespace internal
{
template<class, class>
struct TL;

template<class T, class N = TL<void, void>>
struct TL
{
    using type = T;
    using next = N;
};

template<>
struct TL<void, void>
{
    using type = void;
    using next = void;
};

template<class L, class T, size_t ID = 0U>
struct TL_id
{
    using recurse = TL_id<typename L::next, T, ID + 1U>;
    using type    = T;

    enum V
    {
        // clang-format off
        value = std::is_same<
            typename std::remove_const<typename L::type>::type,
            typename std::remove_const<T>::type
        >::value
            ? ID
            : static_cast<size_t>(recurse::value),

        value_const = std::is_same<typename L::type,T>::value
            ? ID
            : static_cast<size_t>(recurse::value_const)
        // clang-format on
    };
};

template<typename T, bool SAME_ID>
struct actual_T;

template<typename T>
struct actual_T<T, true>
{
    using type = T;
};

template<typename T>
struct actual_T<T, false>
{
    using type = T const;
};

template<typename TL_ID>
struct return_T
{
    using type = typename actual_T<typename TL_ID::type, TL_ID::value == TL_ID::value_const>::type;
};

template<class T, size_t ID>
struct TL_id<void, T, ID>
{
    enum
    {
        value       = 0xFFFF,
        value_const = 0xFFFF
    };
};

template<class L>
struct TL_id<L, void>
{
    enum
    {
        value       = 0xFFFF,
        value_const = 0xFFFF
    };
};

template<class L, class = typename L::type>
struct TL_count
{
    using T       = typename L::type;
    using recurse = TL_count<typename L::next>;

    enum V
    {
        value = 1 + recurse::value
    };
};

template<class L>
struct TL_count<L, void>
{
    enum
    {
        value = 0
    };
};

template<typename T, typename MS>
struct multi_to
{
    static slice<T> sl(MS const& ms) { return ms.template get<T>(); }
};

template<typename MS>
struct multi_to<void, MS>
{
    static slice<void> sl(MS const&) { return slice<void>(); }
};

} // namespace internal

/**
 * Maintain a set of equally sized slices
 */
template<
    class T0, // We should always have at least two types
    class T1, // Assumed by implementation in several places
    class T2 = void,
    class T3 = void,
    class T4 = void,
    class T5 = void,
    class T6 = void,
    class T7 = void>
class multi_slice
{
public:
    using types = internal::TL<
        T0,
        internal::TL<
            T1,
            internal::TL<
                T2,
                internal::TL<
                    T3,
                    internal::TL<T4, internal::TL<T5, internal::TL<T6, internal::TL<T7>>>>>>>>;

private:
    // Ensure type uniqueness
    static_assert(internal::TL_id<types, T0>::value >= 0, "");
    static_assert(internal::TL_id<types, T1>::value >= 1, "");
    static_assert(internal::TL_id<types, T2>::value >= 2, "");
    static_assert(internal::TL_id<types, T3>::value >= 3, "");
    static_assert(internal::TL_id<types, T4>::value >= 4, "");
    static_assert(internal::TL_id<types, T5>::value >= 5, "");
    static_assert(internal::TL_id<types, T6>::value >= 6, "");
    static_assert(internal::TL_id<types, T7>::value >= 7, "");

    size_t _size = 0U;
    array<void const*, internal::TL_count<types>::value> _p{};

public:
    multi_slice() = default;

    ~multi_slice() = default; /* special thanks to our favorite compiler vendor */

    template<
        typename O0,
        typename O1,
        typename O2,
        typename O3,
        typename O4,
        typename O5,
        typename O6,
        typename O7>
    multi_slice(multi_slice<O0, O1, O2, O3, O4, O5, O6, O7> const& other)
    {
        multi_slice const tmp(
            internal::multi_to<T0, multi_slice<O0, O1, O2, O3, O4, O5, O6, O7>>::sl(other),
            internal::multi_to<T1, multi_slice<O0, O1, O2, O3, O4, O5, O6, O7>>::sl(other),
            internal::multi_to<T2, multi_slice<O0, O1, O2, O3, O4, O5, O6, O7>>::sl(other),
            internal::multi_to<T3, multi_slice<O0, O1, O2, O3, O4, O5, O6, O7>>::sl(other),
            internal::multi_to<T4, multi_slice<O0, O1, O2, O3, O4, O5, O6, O7>>::sl(other),
            internal::multi_to<T5, multi_slice<O0, O1, O2, O3, O4, O5, O6, O7>>::sl(other),
            internal::multi_to<T6, multi_slice<O0, O1, O2, O3, O4, O5, O6, O7>>::sl(other),
            internal::multi_to<T7, multi_slice<O0, O1, O2, O3, O4, O5, O6, O7>>::sl(other));
        _size = tmp._size;
        _p    = tmp._p;
    }

    multi_slice(multi_slice const& other) : _size(other._size), _p(other._p) {}

    multi_slice(
        slice<T0> const s0,
        slice<T1> const s1,
        slice<T2> const s2 = slice<T2>(),
        slice<T3> const s3 = slice<T3>(),
        slice<T4> const s4 = slice<T4>(),
        slice<T5> const s5 = slice<T5>(),
        slice<T6> const s6 = slice<T6>(),
        slice<T7> const s7 = slice<T7>())
    {
        size_t const size = s0.size();

        size_t const count = internal::TL_count<types>::value;

        // clang-format off
                                                             _p[0] = s0.data();
                        { if (size != s1.size()) { return; } _p[1] = s1.data(); }
        if (count > 2U) { if (size != s2.size()) { return; } _p[2] = s2.data(); }
        if (count > 3U) { if (size != s3.size()) { return; } _p[3] = s3.data(); }
        if (count > 4U) { if (size != s4.size()) { return; } _p[4] = s4.data(); }
        if (count > 5U) { if (size != s5.size()) { return; } _p[5] = s5.data(); }
        if (count > 6U) { if (size != s6.size()) { return; } _p[6] = s6.data(); }
        if (count > 7U) { if (size != s7.size()) { return; } _p[7] = s7.data(); }
        // clang-format on

        _size = size;
    }

    template<typename T>
    size_t idx(T* const v) const
    {
        return static_cast<size_t>(v - data<T>());
    }

    size_t size() const { return _size; }

    template<class T>
    slice<typename internal::return_T<internal::TL_id<types, T>>::type> get() const
    {
        return slice<typename internal::return_T<internal::TL_id<types, T>>::type>::from_pointer(
            data<T>(), _size);
    }

    template<class T>
    typename internal::return_T<internal::TL_id<types, T>>::type* data() const
    {
        // ensure T is in the list
        static_assert(
            (static_cast<size_t>(internal::TL_id<types, T>::value)
             < static_cast<size_t>(internal::TL_count<types>::value)),
            "");

        return reinterpret_cast<typename internal::return_T<internal::TL_id<types, T>>::type*>(
            const_cast<void*>(_p[internal::TL_id<types, T>::value]));
    }

    template<class T>
    typename internal::return_T<internal::TL_id<types, T>>::type& at(size_t const idx) const
    {
        return data<T>()[idx];
    }

    multi_slice& trim(size_t const limit)
    {
        _size = (limit <= size()) ? limit : 0U;
        return *this;
    }

    multi_slice& advance(size_t const offset)
    {
        size_t const by = (offset >= _size) ? _size : offset;
        _size -= by;

        size_t const c = internal::TL_count<types>::value;

        // clang-format off
                   { _p[0] = reinterpret_cast<uint8_t const*>(_p[0]) + (by * size_of<T0>::value); }
                   { _p[1] = reinterpret_cast<uint8_t const*>(_p[1]) + (by * size_of<T1>::value); }
        if (c > 2) { _p[2] = reinterpret_cast<uint8_t const*>(_p[2]) + (by * size_of<T2>::value); }
        if (c > 3) { _p[3] = reinterpret_cast<uint8_t const*>(_p[3]) + (by * size_of<T3>::value); }
        if (c > 4) { _p[4] = reinterpret_cast<uint8_t const*>(_p[4]) + (by * size_of<T4>::value); }
        if (c > 5) { _p[5] = reinterpret_cast<uint8_t const*>(_p[5]) + (by * size_of<T5>::value); }
        if (c > 6) { _p[6] = reinterpret_cast<uint8_t const*>(_p[6]) + (by * size_of<T6>::value); }
        if (c > 7) { _p[7] = reinterpret_cast<uint8_t const*>(_p[7]) + (by * size_of<T7>::value); }
        // clang-format on
        return *this;
    }
};

template<class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
multi_slice<T0, T1, T2, T3, T4, T5, T6, T7> make_multi_slice(
    slice<T0> const s0,
    slice<T1> const s1,
    slice<T2> const s2,
    slice<T3> const s3,
    slice<T4> const s4,
    slice<T5> const s5,
    slice<T6> const s6,
    slice<T7> const s7)
{
    return multi_slice<T0, T1, T2, T3, T4, T5, T6, T7>(s0, s1, s2, s3, s4, s5, s6, s7);
}

template<class T0, class T1>
multi_slice<T0, T1> make_multi_slice(slice<T0> const s0, slice<T1> const s1)
{
    return make_multi_slice(
        s0,
        s1,
        slice<void>(),
        slice<void>(),
        slice<void>(),
        slice<void>(),
        slice<void>(),
        slice<void>());
}

template<class T0, class T1, class T2>
multi_slice<T0, T1, T2> make_multi_slice(slice<T0> const s0, slice<T1> const s1, slice<T2> const s2)
{
    return make_multi_slice(
        s0, s1, s2, slice<void>(), slice<void>(), slice<void>(), slice<void>(), slice<void>());
}

template<class T0, class T1, class T2, class T3>
multi_slice<T0, T1, T2, T3>
make_multi_slice(slice<T0> const s0, slice<T1> const s1, slice<T2> const s2, slice<T3> const s3)
{
    return make_multi_slice(
        s0, s1, s2, s3, slice<void>(), slice<void>(), slice<void>(), slice<void>());
}

template<class T0, class T1, class T2, class T3, class T4>
multi_slice<T0, T1, T2, T3, T4> make_multi_slice(
    slice<T0> const s0,
    slice<T1> const s1,
    slice<T2> const s2,
    slice<T3> const s3,
    slice<T4> const s4)
{
    return make_multi_slice(s0, s1, s2, s3, s4, slice<void>(), slice<void>(), slice<void>());
}

template<class T0, class T1, class T2, class T3, class T4, class T5>
multi_slice<T0, T1, T2, T3, T4, T5> make_multi_slice(
    slice<T0> const s0,
    slice<T1> const s1,
    slice<T2> const s2,
    slice<T3> const s3,
    slice<T4> const s4,
    slice<T5> const s5)
{
    return make_multi_slice(s0, s1, s2, s3, s4, s5, slice<void>(), slice<void>());
}

} // namespace estd

