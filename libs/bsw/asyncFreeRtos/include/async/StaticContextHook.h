// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "estd/singleton.h"

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
class StaticContextHook : public ::estd::singleton<T>
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
StaticContextHook<T>::StaticContextHook(T& instance) : ::estd::singleton<T>(instance)
{}

template<class T>
inline void StaticContextHook<T>::enterTask(size_t const taskIdx)
{
    ::estd::singleton<T>::instance().enterTask(taskIdx);
}

template<class T>
inline void StaticContextHook<T>::leaveTask(size_t const taskIdx)
{
    ::estd::singleton<T>::instance().leaveTask(taskIdx);
}

template<class T>
inline void StaticContextHook<T>::enterIsrGroup(size_t const isrGroupIdx)
{
    ::estd::singleton<T>::instance().enterIsrGroup(isrGroupIdx);
}

template<class T>
inline void StaticContextHook<T>::leaveIsrGroup(size_t const isrGroupIdx)
{
    ::estd::singleton<T>::instance().leaveIsrGroup(isrGroupIdx);
}

} // namespace async

