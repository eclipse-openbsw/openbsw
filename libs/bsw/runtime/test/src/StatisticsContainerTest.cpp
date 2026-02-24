// Copyright 2024 Accenture.

#include "runtime/StatisticsContainer.h"

#include <etl/span.h>
#include <util/estd/gtest_extensions.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;
using namespace ::runtime;
using namespace ::estd::test;

// NOLINTBEGIN(cert-oop54-cpp): This is intentional for testing purposes.
// NOLINTBEGIN(bugprone-unhandled-self-assignment): This is intentional for testing purposes.
struct TestStatistics
{
    TestStatistics& operator=(TestStatistics const& src)
    {
        _assignedFrom = &src;
        return *this;
    }

    TestStatistics const* _assignedFrom = nullptr;
};

// NOLINTEND(bugprone-unhandled-self-assignment)
// NOLINTEND(cert-oop54-cpp)

struct TestEntry : public TestStatistics
{};

class TestNames
{
public:
    explicit TestNames(::etl::span<char const*> const& names) : _names(names) {}

    char const* getName(size_t idx) const { return _names[idx]; }

private:
    ::etl::span<char const*> _names;
};

TEST(StatisticsContainerTest, testStatisticsArray)
{
    using CutType                 = StatisticsContainer<TestStatistics>;
    char const* statisticsNames[] = {
        nullptr,
        "abc",
        nullptr,
        "def",
        nullptr,
    };
    static size_t const ENTRY_COUNT = sizeof(statisticsNames) / sizeof(statisticsNames[0]);
    TestNames names(statisticsNames);
    CutType::EntryType entries[ENTRY_COUNT];
    CutType cut(entries, CutType::GetNameType::create<TestNames, &TestNames::getName>(names));
    {
        // test all values
        EXPECT_EQ(statisticsNames[0U], cut.getName(0U));
        EXPECT_EQ(&entries[0U], &cut.getStatistics(0U));
        EXPECT_EQ(&entries[0U], &cut.getEntry(0U));
        EXPECT_EQ(statisticsNames[1U], cut.getName(1U));
        EXPECT_EQ(&entries[1U], &cut.getStatistics(1U));
        EXPECT_EQ(&entries[1U], &cut.getEntry(1U));
        EXPECT_EQ(statisticsNames[2U], cut.getName(2U));
        EXPECT_EQ(&entries[2U], &cut.getStatistics(2U));
        EXPECT_EQ(&entries[2U], &cut.getEntry(2U));
        EXPECT_EQ(statisticsNames[3U], cut.getName(3U));
        EXPECT_EQ(&entries[3U], &cut.getStatistics(3U));
        EXPECT_EQ(&entries[3U], &cut.getEntry(3U));
        EXPECT_EQ(statisticsNames[4U], cut.getName(4U));
        EXPECT_EQ(&entries[4U], &cut.getStatistics(4U));
        EXPECT_EQ(&entries[4U], &cut.getEntry(4U));
        // test entries
        EXPECT_THAT(cut.getEntries(), Slice(entries, ENTRY_COUNT));
    }
    StatisticsContainer<TestStatistics>::IteratorType it = cut.getIterator();
    {
        // expect entry 1 as initial entry
        EXPECT_TRUE(it.hasValue());
        EXPECT_EQ(statisticsNames[1U], it.getName());
        EXPECT_EQ(&entries[1U], &it.getStatistics());
    }
    {
        // expect entry 3 as next value
        it.next();
        EXPECT_TRUE(it.hasValue());
        EXPECT_EQ(statisticsNames[3U], it.getName());
        EXPECT_EQ(&entries[3U], &it.getStatistics());
    }
    {
        // expect no more value
        it.next();
        EXPECT_FALSE(it.hasValue());
    }
}

TEST(StatisticsContainerTest, testEntryArray)
{
    using CutType                 = StatisticsContainer<TestStatistics, TestEntry>;
    char const* statisticsNames[] = {
        nullptr,
        "abc",
        nullptr,
        "def",
        nullptr,
    };
    TestNames names(statisticsNames);
    CutType::EntryType entries[sizeof(statisticsNames) / sizeof(statisticsNames[0])];
    CutType cut(entries, CutType::GetNameType::create<TestNames, &TestNames::getName>(names));
    {
        // test all values
        EXPECT_EQ(statisticsNames[0U], cut.getName(0U));
        EXPECT_EQ(&entries[0U], &cut.getStatistics(0U));
        EXPECT_EQ(&entries[0U], &cut.getEntry(0U));
        EXPECT_EQ(statisticsNames[1U], cut.getName(1U));
        EXPECT_EQ(&entries[1U], &cut.getStatistics(1U));
        EXPECT_EQ(&entries[1U], &cut.getEntry(1U));
        EXPECT_EQ(statisticsNames[2U], cut.getName(2U));
        EXPECT_EQ(&entries[2U], &cut.getStatistics(2U));
        EXPECT_EQ(&entries[2U], &cut.getEntry(2U));
        EXPECT_EQ(statisticsNames[3U], cut.getName(3U));
        EXPECT_EQ(&entries[3U], &cut.getStatistics(3U));
        EXPECT_EQ(&entries[3U], &cut.getEntry(3U));
        EXPECT_EQ(statisticsNames[4U], cut.getName(4U));
        EXPECT_EQ(&entries[4U], &cut.getStatistics(4U));
        EXPECT_EQ(&entries[4U], &cut.getEntry(4U));
    }
    StatisticsContainer<TestStatistics, TestEntry>::IteratorType it = cut.getIterator();
    {
        // expect entry 1 as initial entry
        EXPECT_TRUE(it.hasValue());
        EXPECT_EQ(statisticsNames[1U], it.getName());
        EXPECT_EQ(&entries[1U], &it.getStatistics());
    }
    {
        // expect entry 3 as next value
        it.next();
        EXPECT_TRUE(it.hasValue());
        EXPECT_EQ(statisticsNames[3U], it.getName());
        EXPECT_EQ(&entries[3U], &it.getStatistics());
    }
    {
        // expect no more value
        it.next();
        EXPECT_FALSE(it.hasValue());
    }
    {
        // expect no more value
        it.next();
        EXPECT_FALSE(it.hasValue());
    }
    {
        // expect entry 1 after reset
        it.reset();
        EXPECT_TRUE(it.hasValue());
        EXPECT_EQ(statisticsNames[1U], it.getName());
        EXPECT_EQ(&entries[1U], &it.getStatistics());
    }
    {
        // copy statistics into a pure container
        TestStatistics destEntries[6];
        StatisticsContainer<TestStatistics> dest(destEntries);
        dest.copyFrom(cut);
        {
            // test all values
            EXPECT_EQ(dest.getName(0U), cut.getName(0U));
            EXPECT_EQ(destEntries[0]._assignedFrom, &cut.getStatistics(0U));
            EXPECT_EQ(dest.getName(1U), cut.getName(1U));
            EXPECT_EQ(destEntries[1]._assignedFrom, &cut.getStatistics(1U));
            EXPECT_EQ(dest.getName(2U), cut.getName(2U));
            EXPECT_EQ(destEntries[2]._assignedFrom, &cut.getStatistics(2U));
            EXPECT_EQ(dest.getName(3U), cut.getName(3U));
            EXPECT_EQ(destEntries[3]._assignedFrom, &cut.getStatistics(3U));
            EXPECT_EQ(dest.getName(4U), cut.getName(4U));
            EXPECT_EQ(destEntries[4]._assignedFrom, &cut.getStatistics(4U));
        }
    }
    {
        TestStatistics destEntries[4];
        StatisticsContainer<TestStatistics> dest(destEntries);
        EXPECT_THROW({ dest.copyFrom(cut); }, ::etl::exception);
    }
}

TEST(StatisticsContainerTest, testDefaultConstructor)
{
    // default
    ::runtime::StatisticsContainer<TestStatistics> cut;
    EXPECT_EQ(0U, cut.getSize());
}

TEST(StatisticsContainerTest, testDeclare)
{
    {
        // default
        ::runtime::declare::StatisticsContainer<TestStatistics, 3U> cut;
        EXPECT_EQ(3U, cut.getSize());
    }
    {
        using CutType                 = ::runtime::declare::StatisticsContainer<TestStatistics, 4U>;
        // with names
        char const* statisticsNames[] = {nullptr, "abc", nullptr, "test"};
        TestNames names(statisticsNames);
        CutType cut(CutType::GetNameType::create<TestNames, &TestNames::getName>(names));
        EXPECT_EQ(4U, cut.getSize());
    }
}
} // namespace
