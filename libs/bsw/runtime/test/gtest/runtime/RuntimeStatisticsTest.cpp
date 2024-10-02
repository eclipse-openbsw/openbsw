// Copyright 2024 Accenture.

#include "runtime/RuntimeStatistics.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;
using namespace ::runtime;

TEST(RuntimeStatisticsTest, testConstructor)
{
    RuntimeStatistics cut;
    EXPECT_EQ(0U, cut.getTotalRuntime());
    EXPECT_EQ(0U, cut.getTotalRunCount());
    EXPECT_EQ(0U, cut.getMinRuntime());
    EXPECT_EQ(0U, cut.getMaxRuntime());
    EXPECT_EQ(0U, cut.getAverageRuntime());
}

TEST(RuntimeStatisticsTest, testAddRun)
{
    RuntimeStatistics cut;
    {
        cut.addRun(15U);
        EXPECT_EQ(15U, cut.getTotalRuntime());
        EXPECT_EQ(1U, cut.getTotalRunCount());
        EXPECT_EQ(15U, cut.getMinRuntime());
        EXPECT_EQ(15U, cut.getMaxRuntime());
        EXPECT_EQ(15U, cut.getAverageRuntime());
    }
    {
        cut.addRun(30U);
        EXPECT_EQ(45U, cut.getTotalRuntime());
        EXPECT_EQ(2U, cut.getTotalRunCount());
        EXPECT_EQ(15U, cut.getMinRuntime());
        EXPECT_EQ(30U, cut.getMaxRuntime());
        EXPECT_EQ(22U, cut.getAverageRuntime());
    }
    {
        cut.addRun(10U);
        EXPECT_EQ(55U, cut.getTotalRuntime());
        EXPECT_EQ(3U, cut.getTotalRunCount());
        EXPECT_EQ(10U, cut.getMinRuntime());
        EXPECT_EQ(30U, cut.getMaxRuntime());
        EXPECT_EQ(18U, cut.getAverageRuntime());
    }
    {
        // full-blown addRun
        cut.addRun(500U, 20U, 7500U);
        EXPECT_EQ(75U, cut.getTotalRuntime());
        EXPECT_EQ(4U, cut.getTotalRunCount());
        EXPECT_EQ(10U, cut.getMinRuntime());
        EXPECT_EQ(30U, cut.getMaxRuntime());
        EXPECT_EQ(18U, cut.getAverageRuntime());
    }
}

TEST(RuntimeStatisticsTest, testReset)
{
    RuntimeStatistics cut;
    {
        cut.addRun(15U);
        EXPECT_EQ(15U, cut.getTotalRuntime());
        EXPECT_EQ(1U, cut.getTotalRunCount());
        EXPECT_EQ(15U, cut.getMinRuntime());
        EXPECT_EQ(15U, cut.getMaxRuntime());
        EXPECT_EQ(15U, cut.getAverageRuntime());
    }
    cut.reset();
    EXPECT_EQ(0U, cut.getTotalRuntime());
    EXPECT_EQ(0U, cut.getTotalRunCount());
    EXPECT_EQ(0U, cut.getMinRuntime());
    EXPECT_EQ(0U, cut.getMaxRuntime());
    EXPECT_EQ(0U, cut.getAverageRuntime());
}

} // namespace
