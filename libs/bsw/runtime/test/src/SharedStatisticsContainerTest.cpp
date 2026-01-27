// Copyright 2024 Accenture.

#include "runtime/SharedStatisticsContainer.h"

#include "runtime/StatisticsContainer.h"

#include <etl/span.h>
#include <etl/string.h>
#include <util/estd/gtest_extensions.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;
using namespace ::runtime;
using namespace ::estd::test;

// NOTLINTBEGIN(bugprone-unhandled-self-assignment): This is intentional for testing purposes.
struct TestStatistics
{
    TestStatistics& operator=(TestStatistics const& src)
    {
        _assignedFrom = &src;
        return *this;
    }

    TestStatistics const* _assignedFrom = nullptr;
};

// NOTLINTEND(bugprone-unhandled-self-assignment)

struct TestEntry : public TestStatistics
{};

class TestNames
{
public:
    explicit TestNames(::etl::span<char const* const> const& names) : _names(names) {}

    char const* getName(size_t idx) const { return _names[idx]; }

private:
    ::etl::span<char const* const> _names;
};

TEST(SharedStatisticsContainerTest, testCachesNames)
{
    char const* const statisticsNames[] = {
        nullptr,
        "abc",
        nullptr,
        "def",
        nullptr,
    };
    static size_t const ENTRY_COUNT = sizeof(statisticsNames) / sizeof(statisticsNames[0]);
    TestNames names(statisticsNames);

    using SourceType = StatisticsContainer<TestStatistics>;
    using CutType    = SharedStatisticsContainer<TestStatistics, ENTRY_COUNT>;

    SourceType::EntryType entries[ENTRY_COUNT];
    SourceType source{entries, CutType::GetNameType::create<TestNames, &TestNames::getName>(names)};
    CutType cut{};

    cut.copyFrom(source);

    {
        auto it = cut.getIterator();
        {
            // expect entry 1 as initial entry
            EXPECT_TRUE(it.hasValue());
            ::etl::string<8> originalName = statisticsNames[1U];
            ::etl::string<8> cachedName   = it.getName();
            EXPECT_EQ(originalName, cachedName);
            EXPECT_EQ(&entries[1U], it.getStatistics()._assignedFrom);
        }
        {
            // expect entry 3 as next value
            it.next();
            EXPECT_TRUE(it.hasValue());
            ::etl::string<8> originalName = statisticsNames[3U];
            ::etl::string<8> cachedName   = it.getName();
            EXPECT_EQ(originalName, cachedName);
            EXPECT_EQ(&entries[3U], it.getStatistics()._assignedFrom);
        }
        {
            // expect no more value
            it.next();
            EXPECT_FALSE(it.hasValue());
        }
    }

    // New calls to copyFrom from don't access the names (they are cached)
    char const* const changedStatisticsNames[] = {
        "one",
        "two",
        "three",
        "four",
        "five",
    };
    TestNames changedNames(changedStatisticsNames);
    SourceType::EntryType changedEntries[ENTRY_COUNT];
    SourceType changedSource{
        changedEntries, CutType::GetNameType::create<TestNames, &TestNames::getName>(changedNames)};

    cut.copyFrom(changedSource);

    // expectations are same as before for names, but updated stats
    {
        auto it = cut.getIterator();
        {
            // expect entry 1 as initial entry
            EXPECT_TRUE(it.hasValue());
            ::etl::string<8> originalName = statisticsNames[1U];
            ::etl::string<8> cachedName   = it.getName();
            EXPECT_EQ(originalName, cachedName);
            EXPECT_EQ(&changedEntries[1U], it.getStatistics()._assignedFrom);
        }
        {
            // expect entry 3 as next value
            it.next();
            EXPECT_TRUE(it.hasValue());
            ::etl::string<8> originalName = statisticsNames[3U];
            ::etl::string<8> cachedName   = it.getName();
            EXPECT_EQ(originalName, cachedName);
            EXPECT_EQ(&changedEntries[3U], it.getStatistics()._assignedFrom);
        }
        {
            // expect no more value
            it.next();
            EXPECT_FALSE(it.hasValue());
        }
    }
}

} // namespace
