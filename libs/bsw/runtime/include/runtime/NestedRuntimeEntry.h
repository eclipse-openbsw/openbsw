// Copyright 2024 Accenture.

/**
 * \ingroup runtime
 */
#pragma once

#include "runtime/RuntimeStack.h"
#include "runtime/RuntimeStackEntry.h"

namespace runtime
{
template<class Entry, class Statistics, bool CutOut, class Nested>
class NestedRuntimeEntry
: public RuntimeStackEntry<Entry, CutOut>
, public RuntimeStack<Nested>
, public Statistics
{
public:
    using StatisticsType = Statistics;

    void push(uint32_t now, Entry* prevEntry);
    Entry* pop(uint32_t now);

    void addSuspendedTime(uint32_t suspendedTime);
};

/**
 * Inline implementations.
 */
template<class Entry, class Statistics, bool CutOut, class Nested>
inline void NestedRuntimeEntry<Entry, Statistics, CutOut, Nested>::push(
    uint32_t const now, Entry* const prevEntry)
{
    RuntimeStack<Nested>::resume(now);
    RuntimeStackEntry<Entry, CutOut>::push(now, prevEntry);
}

template<class Entry, class Statistics, bool CutOut, class Nested>
inline Entry* NestedRuntimeEntry<Entry, Statistics, CutOut, Nested>::pop(uint32_t const now)
{
    RuntimeStack<Nested>::suspend(now);
    return RuntimeStackEntry<Entry, CutOut>::pop(now);
}

template<class Entry, class Statistics, bool CutOut, class Nested>
inline void NestedRuntimeEntry<Entry, Statistics, CutOut, Nested>::addSuspendedTime(
    uint32_t const suspendedTime)
{
    RuntimeStack<Nested>::addSuspendedTime(suspendedTime);
    RuntimeStackEntry<Entry, CutOut>::addSuspendedTime(suspendedTime);
}

} // namespace runtime
