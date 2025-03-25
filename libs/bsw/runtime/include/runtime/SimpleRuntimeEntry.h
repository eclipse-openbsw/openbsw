// Copyright 2024 Accenture.

/**
 * \ingroup runtime
 */
#pragma once

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

