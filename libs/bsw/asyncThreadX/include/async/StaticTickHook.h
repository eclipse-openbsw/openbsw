// Copyright 2025 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include <etl/singleton_base.h>

namespace async
{
template<class T>
class StaticTickHook : public ::etl::singleton_base<T>
{
public:
    using InstanceType = T;

    StaticTickHook(T& instance);

    static void handleTick();
};

/**
 * Inline implementation.
 */
template<class T>
StaticTickHook<T>::StaticTickHook(T& instance) : ::etl::singleton_base<T>(instance)
{}

template<class T>
inline void StaticTickHook<T>::handleTick()
{
    ::etl::singleton_base<T>::instance().handleTick();
}

} // namespace async
