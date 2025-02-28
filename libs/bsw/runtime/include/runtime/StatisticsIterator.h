// Copyright 2024 Accenture.

/**
 * \ingroup runtime
 */
#ifndef GUARD_96819215_8F7A_41D6_8DCF_26C038283E2E
#define GUARD_96819215_8F7A_41D6_8DCF_26C038283E2E

#include <estd/functional.h>
#include <estd/slice.h>

#include <cstddef>

namespace runtime
{
template<class Statistics>
class StatisticsIterator
{
public:
    using StatisticsType = Statistics;

    using GetNameType = ::estd::function<char const*(size_t)>;

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

#endif // GUARD_96819215_8F7A_41D6_8DCF_26C038283E2E
