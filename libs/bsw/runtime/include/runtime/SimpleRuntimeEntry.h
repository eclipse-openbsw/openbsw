/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
