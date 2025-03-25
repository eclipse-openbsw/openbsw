// Copyright 2024 Accenture.

#pragma once

#include "estd/algorithm.h"
#include "estd/type_traits.h"

#include <platform/estdint.h>

namespace estd
{
struct type_list_end
{
    // compile-time TL helper function base cases
    static constexpr size_t size() { return 0; }

    template<typename U, size_t INDEX>
    static constexpr size_t index_of()
    {
        return INDEX;
    }

    template<typename U>
    static constexpr bool contains()
    {
        return false;
    }

    static constexpr size_t max_size() { return 0; }

    static constexpr size_t max_align() { return 1; }

    template<template<class> class Trait>
    static constexpr bool all_types()
    {
        return true;
    }
};

/**
 * A template meta programming type list.
 * \tparam    HEAD    Head element of the type list.
 * \tparam    TAIL    Tail of the list.
 *
 * A type_list is a recursive data structure, which is composed of a head
 * element and a tail element. The head is a single type, whereas the tail
 * is again a type_list or the termination marker type_list_end.
 *
 * \section    example    Example usage
 * \code
 * using MyTypes = type_list<int,
 *                   type_list<bool,
 *                     type_list<double>>>;
 *
 * // or simpler, via make_type_list helper:
 *
 * using MyTypes = make_type_list<int, bool, double>::type;
 *
 * \endcode
 */

template<typename T, typename TAIL = type_list_end>
struct type_list
{
    using type = T;
    using tail = TAIL;

    static constexpr size_t size() { return 1 + tail::size(); }

    template<typename U, size_t INDEX = 0U>
    static constexpr size_t index_of()
    {
        return (std::is_same<T, U>::value) ? INDEX : tail::template index_of<U, INDEX + 1>();
    }

    template<typename U>
    static constexpr bool contains()
    {
        return (std::is_same<T, U>::value) ? true : tail::template contains<U>();
    }

    static constexpr size_t max_size()
    {
        return ::estd::max<size_t>(size_of<type>::value, tail::max_size());
    }

    static constexpr size_t max_align()
    {
        return ::estd::max<size_t>(align_of<type>::value, tail::max_align());
    }

    template<template<class> class Trait>
    static constexpr bool all_types()
    {
        return ((Trait<type>::value) && (tail::template all_types<Trait>()));
    }
};

template<typename T, typename... Types>
struct make_type_list
{
    using type = type_list<T, typename make_type_list<Types...>::type>;
};

template<typename T>
struct make_type_list<T>
{
    using type = type_list<T>;
};

// Defaulting T to type_list_end (and the specialization below) allows flat_type_list to terminate
template<typename Enable, typename T = type_list_end, typename... Types>
struct flat_type_list_impl
{
    using type = type_list<T, typename flat_type_list_impl<void, Types...>::type>;
};

// Ending base case for TL concatenation recursion
template<>
struct flat_type_list_impl<void, type_list_end>
{
    using type = type_list_end;
};

// Handles type_list concatenation when a type_list is part of a flat_type_list parameter pack
template<template<typename T, typename U> class TL, typename T, typename U, typename... Types>
struct flat_type_list_impl<
    typename std::enable_if<std::is_same<TL<T, U>, type_list<T, U>>::value>::type,
    TL<T, U>,
    Types...>
{
    using type = typename flat_type_list_impl<
        void,
        T,
        typename flat_type_list_impl<void, U, Types...>::type>::type;
};

// Handles the ending case of a nested type_list during concatenation, to throw away the nested
// final type_list_end
template<typename... Types>
struct flat_type_list_impl<void, type_list_end, Types...>
{
    using type = typename flat_type_list_impl<void, Types...>::type;
};

template<typename... Types>
struct flat_type_list
{
    using type = typename flat_type_list_impl<void, Types...>::type;
};

template<typename TL, typename T, size_t INDEX = 0U>
struct index_of;

template<typename HEAD, typename TAIL, typename T, size_t INDEX>
struct index_of<type_list<HEAD, TAIL>, T, INDEX>
{
    enum
    {
        value = index_of<TAIL, T, INDEX + 1U>::value
    };
};

template<typename HEAD, typename TAIL, size_t INDEX>
struct index_of<type_list<HEAD, TAIL>, HEAD, INDEX>
{
    enum
    {
        value = INDEX
    };
};

template<typename T, size_t INDEX>
struct index_of<type_list_end, T, INDEX>
{};

template<typename TL, typename T>
struct contains
{
    enum
    {
        value
        = std::is_same<typename TL::type, T>::value ? 1 : contains<typename TL::tail, T>::value
    };
};

template<typename T>
struct contains<type_list_end, T>
{
    enum
    {
        value = 0
    };
};

template<typename TL>
struct max_size
{
    enum
    {
        value
        = ::estd::max<size_t>(size_of<typename TL::type>::value, max_size<typename TL::tail>::value)
    };
};

template<>
struct max_size<type_list_end>
{
    enum
    {
        value = 0
    };
};

template<typename TL>
struct max_align
{
    enum
    {
        value = ::estd::max<size_t>(
            align_of<typename TL::type>::value, max_align<typename TL::tail>::value)
    };
};

template<>
struct max_align<type_list_end>
{
    enum
    {
        value = 1
    };
};

template<class TL, template<class> class Trait>
struct all_types
{
    enum
    {
        value
        = (((Trait<typename TL::type>::value > 0)
            && (all_types<typename TL::tail, Trait>::value > 0))
               ? 1
               : 0)
    };
};

template<template<class> class Trait>
struct all_types<type_list_end, Trait>
{
    enum
    {
        value = 1
    };
};

} // namespace estd

