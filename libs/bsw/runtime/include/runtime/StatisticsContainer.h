// Copyright 2024 Accenture.

/**
 * \ingroup runtime
 */
#ifndef GUARD_388D33C0_7224_4D1F_B182_87A067C31A86
#define GUARD_388D33C0_7224_4D1F_B182_87A067C31A86

#include "estd/array.h"
#include "runtime/StatisticsIterator.h"

namespace runtime
{
template<class Statistics, class Entry = Statistics>
class StatisticsContainer
{
public:
    using StatisticsType = Statistics;
    using EntryType      = Entry;

    using GetNameType         = ::estd::function<char const*(size_t)>;
    using EntrySliceType      = ::estd::slice<Entry>;
    using ConstEntrySliceType = ::estd::slice<Entry const>;

    class Iterator : public StatisticsIterator<Statistics>
    {
    public:
        Iterator(Iterator const&) = default;

        Iterator(
            StatisticsContainer<Statistics, Entry>::GetNameType const getName,
            ConstEntrySliceType const& entries)
        : StatisticsIterator<Statistics>(getName, entries.size()), _entries(entries)
        {}

    private:
        Statistics const& getValue(size_t const idx) override { return _entries[idx]; }

        ConstEntrySliceType _entries;
    };

    using IteratorType = Iterator;

    StatisticsContainer() = default;

    explicit StatisticsContainer(EntrySliceType const& entries) : _entries(entries), _getName() {}

    StatisticsContainer(EntrySliceType const& entries, GetNameType const getName)
    : _entries(entries), _getName(getName)
    {}

    EntrySliceType getEntries() { return _entries; }

    EntryType& getEntry(size_t const idx) { return _entries[idx]; }

    ConstEntrySliceType getEntries() const { return _entries; }

    GetNameType getGetName() const { return _getName; }

    IteratorType getIterator() const { return IteratorType(_getName, _entries); }

    size_t getSize() const { return _entries.size(); }

    char const* getName(size_t const idx) const { return _getName(idx); }

    StatisticsType const& getStatistics(size_t const idx) const { return _entries[idx]; }

    template<class OtherEntry>
    void copyFrom(StatisticsContainer<Entry, OtherEntry> const& src)
    {
        auto const srcEntries = src.getEntries();
        size_t const srcSize  = srcEntries.size();
        estd_assert(_entries.size() >= srcEntries.size());
        for (size_t idx = 0U; idx < srcSize; ++idx)
        {
            _entries[idx] = srcEntries[idx];
        }
        _getName = src.getGetName();
    }

    void reset()
    {
        for (auto& entry : _entries)
        {
            entry.reset();
        }
    }

private:
    EntrySliceType _entries{};
    GetNameType _getName;
};

namespace declare
{
template<class Statistics, size_t N, class Entry = Statistics>
class StatisticsContainer : public ::runtime::StatisticsContainer<Statistics, Entry>
{
public:
    using GetNameType = typename ::runtime::StatisticsContainer<Statistics, Entry>::GetNameType;

    explicit StatisticsContainer()
    : ::runtime::StatisticsContainer<Statistics, Entry>(_entries), _entries()
    {}

    explicit StatisticsContainer(GetNameType const getName)
    : ::runtime::StatisticsContainer<Statistics, Entry>(_entries, getName), _entries()
    {}

private:
    ::estd::array<Entry, N> _entries;
};

} // namespace declare
} // namespace runtime

#endif // GUARD_388D33C0_7224_4D1F_B182_87A067C31A86
