// Copyright 2024 Accenture.

/**
 * Contains algorithms that are not part of C++98
 * \file
 * \ingroup estl_utils
 */

#pragma once

namespace estd
{
template<class T>
inline constexpr T min(T const lhs, T const rhs)
{
    return (lhs < rhs) ? lhs : rhs;
}

template<class T>
inline constexpr T max(T const lhs, T const rhs)
{
    return (lhs < rhs) ? rhs : lhs;
}

template<class T, class... Ts>
inline constexpr T min(T const a, T const b, Ts const... args)
{
    return min(a < b ? a : b, args...);
}

template<class T, class... Ts>
inline constexpr T max(T const a, T const b, Ts const... args)
{
    return max(a < b ? b : a, args...);
}

/**
 * Compile-time version of max(), for compatibility with c++03
 */
template<class T, T a, T b>
struct maximum
{
    static T const value = (a > b) ? a : b;
};

/**
 * Compile-time version of min(), for compatibility with c++03
 */
template<class T, T a, T b>
struct minimum
{
    static T const value = (a < b) ? a : b;
};

/**
 * swaps two values
 */
template<typename T>
inline void swap(T& t1, T& t2)
{
    T const temp = t1;
    t1           = t2;
    t2           = temp;
}

/**
 * equals unary class.
 */
template<class T>
struct equals
{
    explicit equals(T const item) : _item(item) {}

    bool operator()(T const& other) const { return _item == other; }

private:
    T _item;
};

/**
 * not_equals unary class.
 */
template<class T>
struct not_equals
{
    explicit not_equals(T const item) : _item(item) {}

    bool operator()(T const& other) const { return !(_item == other); }

private:
    T _item;
};

/**
 * greater unary class.
 */
template<class T>
struct greater1
{
    explicit greater1(T const item) : _item(item) {}

    bool operator()(T const& other) const { return _item < other; }

private:
    T _item;
};

/**
 * less unary class.
 */
template<class T>
struct less1
{
    explicit less1(T const item) : _item(item) {}

    bool operator()(T const& other) const { return other < _item; }

private:
    T _item;
};

template<class InputIterator, class UnaryPredicate>
bool all_of(InputIterator first, InputIterator last, UnaryPredicate pred);

template<class InputIterator, class UnaryPredicate>
bool any_of(InputIterator first, InputIterator last, UnaryPredicate pred);

template<class InputIterator, class UnaryPredicate>
bool none_of(InputIterator first, InputIterator last, UnaryPredicate pred);

/*
 *
 * Implementation
 *
 */

template<class InputIterator, class UnaryPredicate>
bool all_of(InputIterator first, InputIterator const last, UnaryPredicate const pred)
{
    while (first != last)
    {
        bool const result = pred(*first);
        if (!result)
        {
            return false;
        }

        ++first;
    }

    return true;
}

template<class InputIterator, class UnaryPredicate>
bool any_of(InputIterator first, InputIterator const last, UnaryPredicate const pred)
{
    while (first != last)
    {
        if (pred(*first))
        {
            return true;
        }
        ++first;
    }

    return false;
}

template<class InputIterator, class UnaryPredicate>
bool none_of(InputIterator first, InputIterator const last, UnaryPredicate const pred)
{
    while (first != last)
    {
        if (pred(*first))
        {
            return false;
        }
        ++first;
    }
    return true;
}

} // namespace estd
