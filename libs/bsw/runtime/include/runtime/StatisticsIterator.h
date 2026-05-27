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

#include <etl/delegate.h>

#include <cstddef>

namespace runtime
{
template<class Statistics>
class StatisticsIterator
{
public:
    using StatisticsType = Statistics;

    using GetNameType = ::etl::delegate<char const*(size_t)>;

    StatisticsIterator(GetNameType const getName, size_t const valueCount)
    : _getName(getName), _valueCount(valueCount)
    {
        moveToNext();
    }

    StatisticsIterator(StatisticsIterator const&) = default;

    void next()
    {
        if (_currentIdx < _valueCount)
        {
            ++_currentIdx;
            moveToNext();
        }
    }

    void reset()
    {
        _currentIdx = 0U;
        moveToNext();
    }

    bool hasValue() const { return _currentIdx < _valueCount; }

    char const* getName() const { return _getName(_currentIdx); }

    StatisticsType const& getStatistics() { return getValue(_currentIdx); }

private:
    StatisticsIterator& operator=(StatisticsIterator const&) = default;

    virtual StatisticsType const& getValue(size_t idx) = 0;

    void moveToNext()
    {
        while (_currentIdx < _valueCount)
        {
            if (_getName(_currentIdx) != nullptr)
            {
                return;
            }
            ++_currentIdx;
        }
    }

    GetNameType _getName;
    size_t _currentIdx = 0;
    size_t _valueCount;
};

} // namespace runtime
