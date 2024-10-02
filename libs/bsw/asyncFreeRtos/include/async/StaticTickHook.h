// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_7EF028C0_E8D7_4AF1_BA19_F09ECA423071
#define GUARD_7EF028C0_E8D7_4AF1_BA19_F09ECA423071

#include "estd/singleton.h"

namespace async
{
template<class T>
class StaticTickHook : public ::estd::singleton<T>
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
StaticTickHook<T>::StaticTickHook(T& instance) : ::estd::singleton<T>(instance)
{}

template<class T>
inline void StaticTickHook<T>::handleTick()
{
    ::estd::singleton<T>::instance().handleTick();
}

} // namespace async

#endif // GUARD_7EF028C0_E8D7_4AF1_BA19_F09ECA423071
