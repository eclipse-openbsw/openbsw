// Copyright 2024 Accenture.

#include "runtime/StatisticsIterator.h"

#include <etl/span.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;
using namespace ::runtime;

struct TestStatistics
{};

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

class TestIterator : public StatisticsIterator<TestStatistics>
{
public:
    TestIterator(
        StatisticsIterator<TestStatistics>::GetNameType getName,
        ::etl::span<TestStatistics const> const& values)
    : StatisticsIterator<TestStatistics>(getName, values.size()), _values(values)
    {}

protected:
    TestStatistics const& getValue(size_t const idx) override { return _values[idx]; }

private:
    ::etl::span<TestStatistics const> _values;
};

TEST(StatisticsIteratorTest, testArray)
{
    char const* statisticsNames[] = {
        nullptr,
        "abc",
        nullptr,
        "def",
        nullptr,
    };
    TestStatistics entries[sizeof(statisticsNames) / sizeof(statisticsNames[0])];
    TestNames names(statisticsNames);
    TestIterator cut(
        StatisticsIterator<TestStatistics>::GetNameType::create<TestNames, &TestNames::getName>(
            names),
        entries);
    {
        // expect entry 1 as initial entry
        EXPECT_TRUE(cut.hasValue());
        EXPECT_EQ(statisticsNames[1U], cut.getName());
        EXPECT_EQ(&entries[1U], &cut.getStatistics());
    }
    {
        // expect entry 3 as next value
        cut.next();
        EXPECT_TRUE(cut.hasValue());
        EXPECT_EQ(statisticsNames[3U], cut.getName());
        EXPECT_EQ(&entries[3U], &cut.getStatistics());
    }
    {
        // expect no more value
        cut.next();
        EXPECT_FALSE(cut.hasValue());
    }
    {
        // expect no more value
        cut.next();
        EXPECT_FALSE(cut.hasValue());
    }
    {
        // expect entry 1 after reset
        cut.reset();
        EXPECT_TRUE(cut.hasValue());
        EXPECT_EQ(statisticsNames[1U], cut.getName());
        EXPECT_EQ(&entries[1U], &cut.getStatistics());
    }
}

} // namespace
