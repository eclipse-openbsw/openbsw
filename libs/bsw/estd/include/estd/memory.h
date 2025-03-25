// Copyright 2024 Accenture.

#pragma once

#include "estd/algorithm.h"
#include "estd/array.h"
#include "estd/constructor.h"
#include "estd/slice.h"

#include <platform/estdint.h>

#include <cstring>
#include <type_traits>

namespace estd
{
namespace memory
{
inline void set(slice<uint8_t> const destination, uint8_t const value)
{
    if (destination.size() > 0)
    {
        (void)::memset(destination.data(), static_cast<int>(value), destination.size());
    }
}

/**
 * Copy data from source to destination. And return a slice referring to
 * the copied data in the destination location.
 *
 * destination.size() has to be at least source.size().
 * If the destination is too small, no data will be copied and a zero length slice will be returned.
 * If the source.size() is 0, it trims the destination to 0 but does not try to copy any
 * nullpointers which could cause undefined behavior.
 */
inline slice<uint8_t> copy(slice<uint8_t> destination, slice<uint8_t const> const source)
{
    (void)destination.trim(source.size());
    if (destination.size() > 0)
    {
        (void)::memcpy(destination.data(), source.data(), destination.size());
    }
    return destination;
}

template<typename T>
inline slice<T> move(slice<T> destination, slice<T> const source)
{
    (void)destination.trim(source.size());
    if (destination.size() > 0)
    {
        (void)::memmove(destination.data(), source.data(), destination.size() * sizeof(T));
    }
    return destination;
}

inline bool is_equal(slice<uint8_t const> const a, slice<uint8_t const> const b)
{
    if (a.size() != b.size())
    {
        return false;
    }
    return (a.size() == 0) || (0 == ::memcmp(a.data(), b.data(), a.size()));
}

inline bool starts_with(slice<uint8_t const> const bytes, slice<uint8_t const> const pattern)
{
    return (pattern.size() == 0)
           || ((bytes.size() >= pattern.size())
               && (0 == ::memcmp(bytes.data(), pattern.data(), pattern.size())));
}

inline bool ends_with(slice<uint8_t const> const bytes, slice<uint8_t const> const pattern)
{
    // clang-format off
    return (pattern.size() == 0)
        || ((bytes.size() >= pattern.size())
           && (0 == ::memcmp(bytes.data() + (bytes.size() - pattern.size()),
                             pattern.data(),
                             pattern.size())));
    // clang-format on
}

inline ptrdiff_t compare(slice<uint8_t const> const a, slice<uint8_t const> const b)
{
    auto const minSize = min(a.size(), b.size());
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

inline void reverse_bytes(slice<uint8_t> const buf)
{
    slice<uint8_t>::iterator lower = buf.begin();
    slice<uint8_t>::iterator upper = buf.end() - 1;
    uint8_t swap;

    while (lower < upper)
    {
        swap     = *lower;
        *lower++ = *upper;
        *upper-- = swap;
    }
}

template<class T>
slice<uint8_t const, sizeof(T)> as_bytes(T const* const value)
{
    return slice<uint8_t const, sizeof(T)>::from_pointer(reinterpret_cast<uint8_t const*>(value));
}

template<class T>
slice<uint8_t, sizeof(T)> as_bytes(T* const value)
{
    return slice<uint8_t, sizeof(T)>::from_pointer(reinterpret_cast<uint8_t*>(value));
}

template<class T>
slice<T> as_slice(T* const value)
{
    return slice<T, 1>::from_pointer(value);
}

template<class T>
::estd::array<uint8_t, sizeof(T)> to_bytes(T const& value)
{
    ::estd::array<uint8_t, sizeof(T)> a;
    (void)::estd::memory::copy(
        a, slice<uint8_t const, sizeof(T)>::from_pointer(reinterpret_cast<uint8_t const*>(&value)));
    return a;
}

/**
 * Converts a given pointer to bytes and size to slice of bytes
 */
inline slice<uint8_t> as_bytes(uint8_t* const data, size_t const size)
{
    return slice<uint8_t>::from_pointer(data, size);
}

/**
 * Converts a given pointer to bytes and size to slice of bytes
 */
inline slice<uint8_t const> as_bytes(uint8_t const* const data, size_t const size)
{
    return slice<uint8_t const>::from_pointer(data, size);
}

/**
 * Converts a given pointer and size to slice
 */
template<class T>
slice<T> as_slice(T* const data, size_t const size)
{
    return slice<T>::from_pointer(data, size);
}

/**
 * Converts a given pointer and size to slice
 */
template<class T>
slice<T const> as_slice(T const* const data, size_t const size)
{
    return slice<T const>::from_pointer(data, size);
}

inline void align(size_t const alignment, slice<uint8_t>& s)
{
    auto const mod = reinterpret_cast<size_t>(s.data()) % alignment;
    if (mod != 0)
    {
        s.advance(alignment - mod);
    }
}

/**
 * Split off and return a slice containing \p n elements of type \p T from the beginning of \p s.
 * The bytes in \p s are <tt>reinterpret_cast</tt> to elements of type \p T.
 * \n
 * Slice \p s is \b always advanced by <tt>sizeof(T) * n</tt> bytes.
 * Thus, if no elements could be split off (due to \p s being too small)
 * \p s will be empty after this operation.
 *
 * \warning Beware of unaligned access since no alignment checks are done by this method.
 *
 * \param n Number of elements
 * \param s Slice of bytes
 *
 * \return Slice containing \p n elements of type \p T. Will be empty if \p s is too small.
 * */
template<typename T>
slice<T const> take(size_t const n, slice<uint8_t const>& s)
{
    static_assert(
        std::is_trivially_destructible<T>::value, "type has to be trivially destructible");
    static_assert(std::is_trivially_copyable<T>::value, "type has to be trivially copyable");
    static_assert(
        std::is_trivially_constructible<T>::value, "type has to be trivially constructible");

    auto const r = s.reinterpret_as<T const>().subslice(n);
    (void)s.advance(sizeof(T) * n);
    return r;
}

/**
 * Split off and return a slice containing \p n elements of type \p T from the beginning of \p s.
 * The bytes in \p s are <tt>reinterpret_cast</tt> to elements of type \p T.
 * \n
 * Slice \p s is \b always advanced by <tt>sizeof(T) * n</tt> bytes.
 * Thus, if no elements could be split off (due to \p s being too small)
 * \p s will be empty after this operation.
 *
 * \warning Beware of unaligned access since no alignment checks are done by this method.
 *
 * \param n Number of elements
 * \param s Slice of bytes
 *
 * \return Slice containing \p n elements of type \p T. Will be empty if \p s is too small.
 * */
template<typename T>
slice<T> take(size_t const n, slice<uint8_t>& s)
{
    static_assert(
        std::is_trivially_destructible<T>::value, "type has to be trivially destructible");
    static_assert(std::is_trivially_copyable<T>::value, "type has to be trivially copyable");
    static_assert(
        std::is_trivially_constructible<T>::value, "type has to be trivially constructible");

    auto const r = s.reinterpret_as<T>().subslice(n);
    (void)s.advance(sizeof(T) * n);
    return r;
}

/**
 * Split off one element of type \p T from the beginning of \p s and return a const reference to it.
 * The respective bytes in \p s are <tt>reinterpret_cast</tt> to an element of type \p T.
 * Slice \p s is advanced by <tt>sizeof(T)</tt> bytes.
 *
 * \warning Ensure that \p s is large enough to hold at least one element of type \p T.
 * Otherwise, out-of-bounds memory will be accessed.
 * */
template<typename T>
T const& take(slice<uint8_t const>& s)
{
    static_assert(
        std::is_trivially_destructible<T>::value, "type has to be trivially destructible");
    static_assert(std::is_trivially_copyable<T>::value, "type has to be trivially copyable");
    static_assert(
        std::is_trivially_constructible<T>::value, "type has to be trivially constructible");

    T const& t = *reinterpret_cast<T const*>(s.data());
    (void)s.advance(sizeof(T));
    return t;
}

/**
 * Split off one element of type \p T from the beginning of \p s and return a reference to it.
 * The respective bytes in \p s are <tt>reinterpret_cast</tt> to an element of type \p T.
 * Slice \p s is advanced by <tt>sizeof(T)</tt> bytes.
 *
 * \warning Ensure that \p s is large enough to hold at least one element of type \p T.
 * Otherwise, out-of-bounds memory will be accessed.
 * */
template<typename T>
T& take(slice<uint8_t>& s)
{
    static_assert(
        std::is_trivially_destructible<T>::value, "type has to be trivially destructible");
    static_assert(std::is_trivially_copyable<T>::value, "type has to be trivially copyable");
    static_assert(
        std::is_trivially_constructible<T>::value, "type has to be trivially constructible");

    T& t = *reinterpret_cast<T*>(s.data());
    (void)s.advance(sizeof(T));
    return t;
}

/**
 * Splits a given slice at an offset by advancing the input slice by offset elements
 * and returning the elements [0, offset[ as a new slice. The offset must be less
 * then size of the slice, otherwise an empty slice will be returned and the input
 * slice will also be trimmed to size zero.
 * \param s     slice to split
 * \param offset position to split the input slice
 * \return slice [0, offset[
 */
template<typename T>
slice<T> split(slice<T>& s, size_t const offset)
{
    auto const r = s.subslice(offset);
    (void)s.advance(offset);
    return r;
}

template<typename T>
constructor<T> emplace(slice<uint8_t>& s)
{
    constructor<T> const c((s.size() >= sizeof(T)) ? s.data() : nullptr);
    (void)s.advance(sizeof(T));
    return c;
}

template<typename T>
slice<T> make(size_t const n, slice<uint8_t>& s)
{
    slice<T> const r = s.reinterpret_as<T>().trim(n);
    for (size_t i = 0U; i < r.size(); ++i)
    {
        (void)new (r.data() + i) T();
    }
    s.advance(sizeof(T) * r.size());
    return r;
}

template<typename T, typename AllocF>
void alloc_and_construct(AllocF const& alloc, ::estd::slice<T>& s, size_t const count)
{
    slice<uint8_t> m = alloc(count * sizeof(T));
    s                = ::estd::memory::make<T>(m.size() / sizeof(T), m);
}

template<typename T>
slice<uint8_t const> destruct(slice<T const> const s)
{
    for (size_t i = 0U; i < s.size(); ++i)
    {
        s[i].~T();
    }
    return s.template reinterpret_as<uint8_t const>();
}

template<typename T>
slice<uint8_t> destruct(slice<T> const s)
{
    for (size_t i = 0U; i < s.size(); ++i)
    {
        s[i].~T();
    }
    return s.template reinterpret_as<uint8_t>();
}

template<typename T>
T take_or(slice<uint8_t const>& s, T const& fallback)
{
    static_assert(
        std::is_trivially_destructible<T>::value, "type has to be trivially destructible");
    static_assert(std::is_trivially_copyable<T>::value, "type has to be trivially copyable");
    static_assert(
        std::is_trivially_constructible<T>::value, "type has to be trivially constructible");

    return (s.size() >= sizeof(T)) ? take<T>(1U, s)[0] : fallback;
}

template<typename T, typename F>
T peek_or_else(slice<uint8_t const> const& s, F const& f)
{
    static_assert(
        std::is_trivially_destructible<T>::value, "type has to be trivially destructible");
    static_assert(std::is_trivially_copyable<T>::value, "type has to be trivially copyable");
    static_assert(
        std::is_trivially_constructible<T>::value, "type has to be trivially constructible");

    return (s.size() >= sizeof(T)) ? (*reinterpret_cast<T const*>(s.data())) : f();
}

template<typename T, typename F>
T take_or_else(slice<uint8_t const>& s, F const& f)
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

} // namespace memory
} // namespace estd
