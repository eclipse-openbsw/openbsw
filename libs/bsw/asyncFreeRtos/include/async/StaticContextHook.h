// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_B00373CC_CBF3_46CD_86E8_6AB0FBC2BDB9
#define GUARD_B00373CC_CBF3_46CD_86E8_6AB0FBC2BDB9

#include "estd/singleton.h"

namespace async
{
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

#endif // GUARD_B00373CC_CBF3_46CD_86E8_6AB0FBC2BDB9
