// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include <etl/singleton_base.h>

namespace async
{
/**
 * A template class implementing static polymorphism
 * for seamless wrapping of any class with corresponding
 * method implementations.
 *
 * \tparam T The underlying type with method implementations.
 */
template<class T>
class StaticContextHook : public ::etl::singleton_base<T>
{
public:
    using InstanceType = T;

    StaticContextHook(T& instance);

    static void enterTask(size_t taskIdx);
    static void leaveTask(size_t taskIdx);

    static void enterIsrGroup(size_t isrGroupIdx);
    static void leaveIsrGroup(size_t isrGroupIdx);
};

/**
 * Inline implementation.
 */
template<class T>
StaticContextHook<T>::StaticContextHook(T& instance) : ::etl::singleton_base<T>(instance)
{}

template<class T>
inline void StaticContextHook<T>::enterTask(size_t const taskIdx)
{
    ::etl::singleton_base<T>::instance().enterTask(taskIdx);
}

template<class T>
inline void StaticContextHook<T>::leaveTask(size_t const taskIdx)
{
    ::etl::singleton_base<T>::instance().leaveTask(taskIdx);
}

template<class T>
inline void StaticContextHook<T>::enterIsrGroup(size_t const isrGroupIdx)
{
    ::etl::singleton_base<T>::instance().enterIsrGroup(isrGroupIdx);
}

template<class T>
inline void StaticContextHook<T>::leaveIsrGroup(size_t const isrGroupIdx)
{
    ::etl::singleton_base<T>::instance().leaveIsrGroup(isrGroupIdx);
}

} // namespace async
