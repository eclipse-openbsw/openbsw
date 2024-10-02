// Copyright 2024 Accenture.

#include "runtime/StatisticsWriter.h"

#include "runtime/RuntimeStatistics.h"
#include "runtime/StatisticsIterator.h"
#include "util/stream/StringBufferOutputStream.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::runtime;
using namespace ::testing;

struct TestRuntimeStatisticsFormatter
{
    void format(StatisticsWriter& writer, RuntimeStatistics const& statistics) const
    {
        writer.writeRuntimePercentage("%", statistics.getTotalRuntime());
        writer.writeRuntime("min", 6U, statistics.getMinRuntime());
        writer.writeRuntime("max", 6U, statistics.getMaxRuntime());
        writer.writeRuntime("avg", 6U, statistics.getAverageRuntime());
    }
};

class StatisticsWriterTest : public Test
{
protected:
    void writeStatistics(StatisticsWriter& writer)
    {
        writer.writeText("text-header", 9U, "ABCDEFG");      // "text-header" "ABCDEFG"
        writer.writeText("text 2", 8U, "abcdef");            // "   text 2  " "abcdef"
        writer.writeNumber("long-number", 4U, 12345U);       // "long-number" "12345"
        writer.writeNumber("#", 6U, 12345U);                 // "          #" "12345"
        writer.writeRuntime("us-header", 5U, 5000U);         // "  us-header" "100 us"
        writer.writeRuntime("us", 6U, 7500U);                // "         us" "150 us"
        writer.writeRuntimePercentage("percentage", 8123U);  // " percentage" "81.23 %"
        writer.writeRuntimePercentage("pct", 10000U);        // "      p  ct" "100.00 %"
        writer.writePercentage("Percentage", 3483U, 10000U); // " Percentage" "34.83 %"
        writer.writePercentage("Pct", 720U, 10000U);         // "        Pct" "7.20 %"
        writer.writeEol();                                   // "         \n" "\n"
    }
};

struct TestNames
{
public:
    explicit TestNames(::estd::slice<char const* const> const& names) : _names(names) {}

    char const* getName(size_t idx) const { return _names[idx]; }

private:
    ::estd::slice<char const* const> _names;
};

class TestIterator : public StatisticsIterator<RuntimeStatistics>
{
public:
    TestIterator(
        StatisticsIterator<RuntimeStatistics>::GetNameType getName,
        ::estd::slice<RuntimeStatistics const> const& values)
    : StatisticsIterator<RuntimeStatistics>(getName, values.size()), _values(values)
    {}

protected:
    RuntimeStatistics const& getValue(size_t const idx) override { return _values[idx]; }

private:
    ::estd::slice<RuntimeStatistics const> _values;
};

TEST_F(StatisticsWriterTest, testHeader)
{
    ::util::stream::declare::StringBufferOutputStream<300U> stream;
    ::util::format::StringWriter writer(stream);
    StatisticsWriter cut(writer, 10000U, 50U);
    cut.setMode(StatisticsWriter::Mode::Type::HEADER);
    writeStatistics(cut);
    EXPECT_STREQ(
        "text-header text 2   long-number      # us-header        us percentage       pct "
        "Percentage       Pct\n",
        stream.getString());
}

TEST_F(StatisticsWriterTest, testLine)
{
    ::util::stream::declare::StringBufferOutputStream<300U> stream;
    ::util::format::StringWriter writer(stream);
    StatisticsWriter cut(writer, 10000U, 50U);
    cut.setMode(StatisticsWriter::Mode::Type::LINE);
    writeStatistics(cut);
    EXPECT_STREQ(
        "------------------------------------------------------------------------------------------"
        "-----------\n",
        stream.getString());
}

TEST_F(StatisticsWriterTest, testValues)
{
    ::util::stream::declare::StringBufferOutputStream<300U> stream;
    ::util::format::StringWriter writer(stream);
    StatisticsWriter cut(writer, 10000U, 50U);
    writeStatistics(cut);
    EXPECT_STREQ(
        "ABCDEFG     abcdef         12345  12345    100 us    150 us    81.23 %  100.00 %    34.83 "
        "%    7.20 %\n",
        stream.getString());
}

TEST_F(StatisticsWriterTest, testPercentageWithoutRuntime)
{
    ::util::stream::declare::StringBufferOutputStream<300U> stream;
    ::util::format::StringWriter writer(stream);
    StatisticsWriter cut(writer, 0U, 50U);
    cut.writePercentage("test", 125U, 0U);
    EXPECT_STREQ("   0.00 %", stream.getString());
}

TEST_F(StatisticsWriterTest, testRuntimePercentageWithoutRuntime)
{
    ::util::stream::declare::StringBufferOutputStream<300U> stream;
    ::util::format::StringWriter writer(stream);
    StatisticsWriter cut(writer, 0U, 50U);
    cut.writeRuntimePercentage("test", 125U);
    EXPECT_STREQ("   0.00 %", stream.getString());
}

TEST_F(StatisticsWriterTest, testFormatGroup)
{
    ::util::stream::declare::StringBufferOutputStream<300U> stream;
    ::util::format::StringWriter writer(stream);
    RuntimeStatistics statistics[4];
    statistics[0].addRun(300U);
    statistics[1].addRun(250U);
    statistics[1].addRun(420U);
    statistics[3].addRun(10U);
    statistics[3].addRun(1010U);
    StatisticsWriter cut(writer, 10000U, 50U);
    char const* const nameArray[4] = {"first", "middle", nullptr, "last"};
    TestNames const names(nameArray);
    TestIterator it(
        StatisticsIterator<RuntimeStatistics>::GetNameType::create<TestNames, &TestNames::getName>(
            names),
        statistics);
    TestRuntimeStatisticsFormatter formatter;
    cut.formatStatisticsGroup(
        ::estd::function<void(StatisticsWriter&, RuntimeStatistics const&)>::
            create<TestRuntimeStatisticsFormatter, &TestRuntimeStatisticsFormatter::format>(
                formatter),
        "task",
        9U,
        it);
    char const* expected = "task              %       min       max       avg\n"
                           "-------------------------------------------------\n"
                           "first        3.00 %      6 us      6 us      6 us\n"
                           "middle       6.70 %      5 us      8 us      6 us\n"
                           "last        10.20 %      0 us     20 us     10 us\n";
    EXPECT_STREQ(expected, stream.getString());
}

} // namespace
