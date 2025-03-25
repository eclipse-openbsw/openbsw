// Copyright 2024 Accenture.

/**
 * Contains uncopyable class and macro
 * \file
 * \ingroup estl_utils
 */
#pragma once

#ifndef UNCOPYABLE

/**
 * Macro to make a class uncopyable that cannot inherit
 * from the uncopyable base class.
 */
#define UNCOPYABLE(T) \
    T(const T&);      \
    T& operator=(const T&)

#endif

namespace estd
{
/**
 * An uncopyable base class.
 * Extend this class to make your class uncopyable.
 *
 * \section uncopyable_example Usage example
 * \code{.cpp}
 * class MyClass
 * : uncopyable // inherit privately from uncopyable
 * {
 *     // implementation of MyClass
 * };\endcode
 *
 */
class uncopyable
{
    uncopyable(uncopyable const&)            = delete;
    uncopyable& operator=(uncopyable const&) = delete;

protected:
    uncopyable() = default;
};

} // namespace estd

