// Copyright 2021 Accenture.

#ifndef GUARD_EEA6A9AC_DC21_4232_AFA4_CF8B3608BF1F
#define GUARD_EEA6A9AC_DC21_4232_AFA4_CF8B3608BF1F

#include "etl/algorithm.h"
#include "etl/array.h"
#include "etl/span.h"

#include <platform/estdint.h>

#include <cstring>
#include <type_traits>

// suppress misra 2.10.5 EOF: names in this module are reused in estd by design.
// suppress misra 14.5.1 EOF: generic functions in associated namespace estd are intended

namespace estd
{
namespace memory
{
inline void set(::etl::span<uint8_t> const destination, uint8_t const value)
{
    if (destination.size() > 0)
    {
        // suppress misra 3.9.2 next_line: use of int to comply with memset API
        (void)::memset(destination.data(), static_cast<int>(value), destination.size());
    }
}

/**
 * Copy data from source to destination. And return a ::etl::span referring to
 * the copied data in the destination location.
 *
 * destination.size() has to be at least source.size().
 * If the destination is too small, no data will be copied and a zero length ::etl::span will be
 * returned. If the source.size() is 0, it trims the destination to 0 but does not try to copy any
 * nullpointers which could cause undefined behavior.
 */
inline ::etl::span<uint8_t>
copy(::etl::span<uint8_t> destination, ::etl::span<uint8_t const> const source)
{
    //(void)destination.trim(source.size());
    destination = destination.subspan(0, source.size());
    if (destination.size() > 0)
    {
        (void)::memcpy(destination.data(), source.data(), destination.size());
    }
    return destination;
}

template<typename T>
inline ::etl::span<T> move(::etl::span<T> destination, ::etl::span<T> const source)
{
    (void)destination.trim(source.size());
    if (destination.size() > 0)
    {
        (void)::memmove(destination.data(), source.data(), destination.size() * sizeof(T));
    }
    return destination;
}

inline bool is_equal(::etl::span<uint8_t const> const a, ::etl::span<uint8_t const> const b)
{
    if (a.size() != b.size())
    {
        return false;
    }
    return (a.size() == 0) || (0 == ::memcmp(a.data(), b.data(), a.size()));
}

inline bool
starts_with(::etl::span<uint8_t const> const bytes, ::etl::span<uint8_t const> const pattern)
{
    return (pattern.size() == 0)
           || ((bytes.size() >= pattern.size())
               && (0 == ::memcmp(bytes.data(), pattern.data(), pattern.size())));
}

inline bool
ends_with(::etl::span<uint8_t const> const bytes, ::etl::span<uint8_t const> const pattern)
{
    //  suppress misra 5.0.15 next_construct: We intend to use pointer arithmetic here
    // clang-format off
    return (pattern.size() == 0)
        || ((bytes.size() >= pattern.size())
           && (0 == ::memcmp(bytes.data() + (bytes.size() - pattern.size()),
                             pattern.data(),
                             pattern.size())));
    // clang-format on
}

inline ptrdiff_t compare(::etl::span<uint8_t const> const a, ::etl::span<uint8_t const> const b)
{
    auto const minSize = ::etl::min(a.size(), b.size());
    if (minSize > 0)
    {
        auto const pointerDiff = static_cast<ptrdiff_t>(::memcmp(a.data(), b.data(), minSize));

        if (pointerDiff != 0)
        {
            return pointerDiff;
        }
    }

    return static_cast<ptrdiff_t>(b.size()) - static_cast<ptrdiff_t>(a.size());
}

inline void reverse_bytes(::etl::span<uint8_t> const buf)
{
    ::etl::span<uint8_t>::iterator lower = buf.begin();
    //  suppress misra 5.0.15 next_construct: We intend to use pointer arithmetic here
    ::etl::span<uint8_t>::iterator upper = buf.end() - 1;
    uint8_t swap;

    while (lower < upper)
    {
        swap     = *lower;
        *lower++ = *upper;
        *upper-- = swap;
    }
}

template<class T>
::etl::span<uint8_t const, sizeof(T)> as_bytes(T const* const value)
{
    return ::etl::span<uint8_t const, sizeof(T)>(reinterpret_cast<uint8_t const*>(value));
}

template<class T>
::etl::span<uint8_t, sizeof(T)> as_bytes(T* const value)
{
    return ::etl::span<uint8_t, sizeof(T)>(reinterpret_cast<uint8_t*>(value), sizeof(T));
}

template<class T>
::etl::span<T> as_span(T* const value)
{
    return ::etl::span<T, 1>(value, 1);
}

template<class T>
::etl::array<uint8_t, sizeof(T)> to_bytes(T const& value)
{
    ::etl::array<uint8_t, sizeof(T)> a;
    (void)::estd::memory::copy(
        a, ::etl::span<uint8_t const, sizeof(T)>(reinterpret_cast<uint8_t const*>(&value)));
    return a;
}

/**
 * Converts a given pointer to bytes and size to ::etl::span of bytes
 */
inline ::etl::span<uint8_t> as_bytes(uint8_t* const data, size_t const size)
{
    return ::etl::span<uint8_t>(data, size);
}

/**
 * Converts a given pointer to bytes and size to ::etl::span of bytes
 */
inline ::etl::span<uint8_t const> as_bytes(uint8_t const* const data, size_t const size)
{
    return ::etl::span<uint8_t const>(data, size);
}

/**
 * Converts a given pointer and size to ::etl::span
 */
template<class T>
::etl::span<T> as_span(T* const data, size_t const size)
{
    return ::etl::span<T>::from_pointer(data, size);
}

/**
 * Converts a given pointer and size to ::etl::span
 */
template<class T>
::etl::span<T const> as_span(T const* const data, size_t const size)
{
    return ::etl::span<T const>::from_pointer(data, size);
}

inline void align(size_t const alignment, ::etl::span<uint8_t>& s)
{
    // suppress misra 5.2.9 next_line: Cast to number required for modulo
    auto const mod = reinterpret_cast<size_t>(s.data()) % alignment;
    if (mod != 0)
    {
        s.advance(alignment - mod);
    }
}

/**
 * Split off and return a ::etl::span containing \p n elements of type \p T from the beginning of \p
 * s. The bytes in \p s are <tt>reinterpret_cast</tt> to elements of type \p T. \n
 * ::etl::span \p s is \b always advanced by <tt>sizeof(T) * n</tt> bytes.
 * Thus, if no elements could be split off (due to \p s being too small)
 * \p s will be empty after this operation.
 *
 * \warning Beware of unaligned access since no alignment checks are done by this method.
 *
 * \param n Number of elements
 * \param s ::etl::span of bytes
 *
 * \return ::etl::span containing \p n elements of type \p T. Will be empty if \p s is too small.
 * */
template<typename T>
::etl::span<T const> take(size_t const n, ::etl::span<uint8_t const>& s)
{
    static_assert(
        std::is_trivially_destructible<T>::value, "type has to be trivially destructible");
    static_assert(std::is_trivially_copyable<T>::value, "type has to be trivially copyable");
    static_assert(
        std::is_trivially_constructible<T>::value, "type has to be trivially constructible");

    auto const r = s.reinterpret_as<T const>().subspan(0, n);
    (void)s.advance(sizeof(T) * n);
    return r;
}

/**
 * Split off and return a ::etl::span containing \p n elements of type \p T from the beginning of \p
 * s. The bytes in \p s are <tt>reinterpret_cast</tt> to elements of type \p T. \n
 * ::etl::span \p s is \b always advanced by <tt>sizeof(T) * n</tt> bytes.
 * Thus, if no elements could be split off (due to \p s being too small)
 * \p s will be empty after this operation.
 *
 * \warning Beware of unaligned access since no alignment checks are done by this method.
 *
 * \param n Number of elements
 * \param s ::etl::span of bytes
 *
 * \return ::etl::span containing \p n elements of type \p T. Will be empty if \p s is too small.
 * */
template<typename T>
::etl::span<T> take(size_t const n, ::etl::span<uint8_t>& s)
{
    static_assert(
        std::is_trivially_destructible<T>::value, "type has to be trivially destructible");
    static_assert(std::is_trivially_copyable<T>::value, "type has to be trivially copyable");
    static_assert(
        std::is_trivially_constructible<T>::value, "type has to be trivially constructible");

    auto const r = s.reinterpret_as<T>().subspan(0, n);
    (void)s.advance(sizeof(T) * n);
    return r;
}

/**
 * Split off one element of type \p T from the beginning of \p s and return a const reference to it.
 * The respective bytes in \p s are <tt>reinterpret_cast</tt> to an element of type \p T.
 * ::etl::span \p s is advanced by <tt>sizeof(T)</tt> bytes.
 *
 * \warning Ensure that \p s is large enough to hold at least one element of type \p T.
 * Otherwise, out-of-bounds memory will be accessed.
 * */
template<typename T>
T const& take(::etl::span<uint8_t const>& s)
{
    static_assert(
        std::is_trivially_destructible<T>::value, "type has to be trivially destructible");
    static_assert(std::is_trivially_copyable<T>::value, "type has to be trivially copyable");
    static_assert(
        std::is_trivially_constructible<T>::value, "type has to be trivially constructible");

    // suppress misra 5.2.7 next_line: cast cannot be avoided
    T const& t = *reinterpret_cast<T const*>(s.data());
    (void)s.advance(sizeof(T));
    return t;
}

/**
 * Split off one element of type \p T from the beginning of \p s and return a reference to it.
 * The respective bytes in \p s are <tt>reinterpret_cast</tt> to an element of type \p T.
 * ::etl::span \p s is advanced by <tt>sizeof(T)</tt> bytes.
 *
 * \warning Ensure that \p s is large enough to hold at least one element of type \p T.
 * Otherwise, out-of-bounds memory will be accessed.
 * */
template<typename T>
T& take(::etl::span<uint8_t>& s)
{
    static_assert(
        std::is_trivially_destructible<T>::value, "type has to be trivially destructible");
    static_assert(std::is_trivially_copyable<T>::value, "type has to be trivially copyable");
    static_assert(
        std::is_trivially_constructible<T>::value, "type has to be trivially constructible");

    // suppress misra 5.2.7 next_line: cast cannot be avoided
    T& t = *reinterpret_cast<T*>(s.data());
    (void)s.advance(sizeof(T));
    return t;
}

/**
 * Splits a given ::etl::span at an offset by advancing the input ::etl::span by offset elements
 * and returning the elements [0, offset[ as a new ::etl::span. The offset must be less
 * then size of the ::etl::span, otherwise an empty ::etl::span will be returned and the input
 * ::etl::span will also be trimmed to size zero.
 * \param s     ::etl::span to split
 * \param offset position to split the input ::etl::span
 * \return ::etl::span [0, offset[
 */
template<typename T>
::etl::span<T> split(::etl::span<T>& s, size_t const offset)
{
    auto const r = s.subspan(0, offset);
    (void)s.advance(offset);
    return r;
}

// template<typename T>
// constructor<T> emplace(::etl::span<uint8_t>& s)
// {
//     constructor<T> const c((s.size() >= sizeof(T)) ? s.data() : nullptr);
//     (void)s.advance(sizeof(T));
//     return c;
// }

template<typename T>
::etl::span<T> make(size_t const n, ::etl::span<uint8_t>& s)
{
    ::etl::span<T> const r = s.reinterpret_as<T>().trim(n);
    for (size_t i = 0U; i < r.size(); ++i)
    {
        // suppress misra 5.0.15 next_line: pointer arithmetic used deliberately
        (void)new (r.data() + i) T();
    }
    s.advance(sizeof(T) * r.size());
    return r;
}

template<typename T, typename AllocF>
void alloc_and_construct(AllocF const& alloc, ::etl::span<T>& s, size_t const count)
{
    ::etl::span<uint8_t> m = alloc(count * sizeof(T));
    s                      = ::estd::memory::make<T>(m.size() / sizeof(T), m);
}

template<typename T>
::etl::span<uint8_t const> destruct(::etl::span<T const> const s)
{
    for (size_t i = 0U; i < s.size(); ++i)
    {
        s[i].~T();
    }
    return s.template reinterpret_as<uint8_t const>();
}

template<typename T>
::etl::span<uint8_t> destruct(::etl::span<T> const s)
{
    for (size_t i = 0U; i < s.size(); ++i)
    {
        s[i].~T();
    }
    return s.template reinterpret_as<uint8_t>();
}

template<typename T>
T take_or(::etl::span<uint8_t const>& s, T const& fallback)
{
    static_assert(
        std::is_trivially_destructible<T>::value, "type has to be trivially destructible");
    static_assert(std::is_trivially_copyable<T>::value, "type has to be trivially copyable");
    static_assert(
        std::is_trivially_constructible<T>::value, "type has to be trivially constructible");

    return (s.size() >= sizeof(T)) ? take<T>(1U, s)[0] : fallback;
}

template<typename T, typename F>
T peek_or_else(::etl::span<uint8_t const> const& s, F const& f)
{
    static_assert(
        std::is_trivially_destructible<T>::value, "type has to be trivially destructible");
    static_assert(std::is_trivially_copyable<T>::value, "type has to be trivially copyable");
    static_assert(
        std::is_trivially_constructible<T>::value, "type has to be trivially constructible");

    return (s.size() >= sizeof(T)) ? (*reinterpret_cast<T const*>(s.data())) : f();
}

template<typename T, typename F>
T take_or_else(::etl::span<uint8_t const>& s, F const& f)
{
    static_assert(
        std::is_trivially_destructible<T>::value, "type has to be trivially destructible");
    static_assert(std::is_trivially_copyable<T>::value, "type has to be trivially copyable");
    static_assert(
        std::is_trivially_constructible<T>::value, "type has to be trivially constructible");

    return (s.size() >= sizeof(T)) ? take<T>(1U, s)[0] : f();
}

template<typename T>
struct unaligned
{
    static_assert(
        std::is_trivially_destructible<T>::value, "type has to be trivially destructible");
    static_assert(std::is_trivially_copyable<T>::value, "type has to be trivially copyable");

    static unaligned<T> make(T const& t)
    {
        unaligned<T> u;
        u.set(t);
        return u;
    }

    uint8_t bytes[sizeof(T)];

    void set(T const& v) { memcpy(bytes, &v, sizeof(T)); }

    T get() const
    {
        T v;
        memcpy(&v, bytes, sizeof(T));
        return v;
    }
};

template<class NewType, class T, size_t Size>
// suppress misra 2.10.5 next_construct: name reinterpret_as is used in multiple specializations
::etl::span<NewType, (sizeof(T) * Size) / sizeof(NewType)>
reinterpret_as(::etl::span<T, Size> const o)
{
    return o.template reinterpret_as<NewType>();
}

// [PUBLIC_NON_MEMBER_API_BEGIN]
inline ::etl::span<uint8_t const> make_str(char const* const s)
{
    size_t i = 0;
    while (s[i] != 0U)
    {
        ++i;
    }
    // suppress misra 5.2.7 next_construct: reinterpret_cast cannot be avoided
    return ::etl::span<uint8_t const>(reinterpret_cast<uint8_t const*>(s), i);
}

/**
 * Create a span from a C array without needing
 * to explicitly specify the type or length.
 * The resulting span will be runtime sized.
 */
template<class T, size_t OtherSize>
constexpr ::etl::span<T, 0> make_span(T (&data)[OtherSize])
{
    return ::etl::span<T, 0>(::etl::span<T, OtherSize>(data, OtherSize).data(), OtherSize);
}

/**
 * Create a span from any contiguous container that provides
 * a .data() and .size() method without needing to explicitly specify the type.
 * The resulting span will be runtime sized.
 */
template<class T>
constexpr ::etl::span<typename T::value_type> make_span(T& data)
{
    return ::etl::span<typename T::value_type>(data);
}

/**
 * Create a span from a C array without needing
 * to explicitly specify the type or length.
 * The resulting span will be statically sized.
 */
template<class T, size_t OtherSize>
constexpr ::etl::span<T, OtherSize> make_static_span(T (&data)[OtherSize])
{
    return ::etl::span<T, OtherSize>(data);
}

/**
 * Create a span from an estd::array without needing
 * to explicitly specify the type or length.
 * The resulting span will be statically sized.
 */
template<class T, size_t OtherSize>
constexpr ::etl::span<T, OtherSize> make_static_span(::etl::array<T, OtherSize>& data)
{
    return ::etl::span<T, OtherSize>(data);
}

/**
 * Create a span from a const ::etl::array without needing
 * to explicitly specify the type or length.
 * The resulting span will be statically sized.
 */
template<class T, size_t OtherSize>
constexpr ::etl::span<T const, OtherSize> make_static_span(::etl::array<T, OtherSize> const& data)
{
    return ::etl::span<T const, OtherSize>(data);
}

} // namespace memory
} // namespace estd

#endif // GUARD_EEA6A9AC_DC21_4232_AFA4_CF8B3608BF1F
