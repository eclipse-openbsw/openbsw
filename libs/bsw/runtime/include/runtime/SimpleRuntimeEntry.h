// Copyright 2024 Accenture.

/**
 * \ingroup runtime
 */
#ifndef GUARD_4C9987EA_4BCF_46A5_B5F6_939234153BB3
#define GUARD_4C9987EA_4BCF_46A5_B5F6_939234153BB3

#include "runtime/RuntimeStackEntry.h"

namespace runtime
{
template<class Entry, class Statistics, bool CutOut>
class SimpleRuntimeEntry
: public RuntimeStackEntry<Entry, CutOut>
, public Statistics
{
public:
    using StatisticsType = Statistics;
};

} // namespace runtime

#endif // GUARD_4C9987EA_4BCF_46A5_B5F6_939234153BB3
