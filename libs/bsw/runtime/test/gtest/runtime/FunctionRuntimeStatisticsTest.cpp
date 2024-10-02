// Copyright 2024 Accenture.

#include "runtime/FunctionRuntimeStatistics.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;
using namespace ::runtime;

TEST(FunctionRuntimeStatisticsTest, testConstructor)
{
    FunctionRuntimeStatistics cut;
    EXPECT_EQ(0U, cut.getTotalRuntime());
    EXPECT_EQ(0U, cut.getTotalRunCount());
    EXPECT_EQ(0U, cut.getMinRuntime());
    EXPECT_EQ(0U, cut.getMaxRuntime());
    EXPECT_EQ(0U, cut.getAverageRuntime());
    EXPECT_EQ(0U, cut.getMinJitter());
    EXPECT_EQ(0U, cut.getMaxJitter());
}

TEST(FunctionRuntimeStatisticsTest, testAddRun)
{
    FunctionRuntimeStatistics cut;
    {
        cut.addRun(25U, 15U);
        EXPECT_EQ(15U, cut.getTotalRuntime());
        EXPECT_EQ(1U, cut.getTotalRunCount());
        EXPECT_EQ(15U, cut.getMinRuntime());
        EXPECT_EQ(15U, cut.getMaxRuntime());
        EXPECT_EQ(15U, cut.getAverageRuntime());
        EXPECT_EQ(0U, cut.getMinJitter());
        EXPECT_EQ(0U, cut.getMaxJitter());
    }
    {
        cut.addRun(45U, 30U);
        EXPECT_EQ(45U, cut.getTotalRuntime());
        EXPECT_EQ(2U, cut.getTotalRunCount());
        EXPECT_EQ(15U, cut.getMinRuntime());
        EXPECT_EQ(30U, cut.getMaxRuntime());
        EXPECT_EQ(22U, cut.getAverageRuntime());
        EXPECT_EQ(20U, cut.getMinJitter());
        EXPECT_EQ(20U, cut.getMaxJitter());
    }
    {
        cut.addRun(85U, 10U);
        EXPECT_EQ(55U, cut.getTotalRuntime());
        EXPECT_EQ(3U, cut.getTotalRunCount());
        EXPECT_EQ(10U, cut.getMinRuntime());
        EXPECT_EQ(30U, cut.getMaxRuntime());
        EXPECT_EQ(18U, cut.getAverageRuntime());
        EXPECT_EQ(20U, cut.getMinJitter());
        EXPECT_EQ(40U, cut.getMaxJitter());
    }
    {
        // full-blown addRun
        cut.addRun(100U, 20U, 7500U);
        EXPECT_EQ(75U, cut.getTotalRuntime());
        EXPECT_EQ(4U, cut.getTotalRunCount());
        EXPECT_EQ(10U, cut.getMinRuntime());
        EXPECT_EQ(30U, cut.getMaxRuntime());
        EXPECT_EQ(18U, cut.getAverageRuntime());
        EXPECT_EQ(15U, cut.getMinJitter());
        EXPECT_EQ(40U, cut.getMaxJitter());
    }
}

TEST(FunctionRuntimeStatisticsTest, testReset)
{
    FunctionRuntimeStatistics cut;
    {
        cut.addRun(20U, 14U);
        cut.addRun(45U, 16U);
        EXPECT_EQ(30U, cut.getTotalRuntime());
        EXPECT_EQ(2U, cut.getTotalRunCount());
        EXPECT_EQ(14U, cut.getMinRuntime());
        EXPECT_EQ(16U, cut.getMaxRuntime());
        EXPECT_EQ(15U, cut.getAverageRuntime());
        EXPECT_EQ(25U, cut.getMinJitter());
        EXPECT_EQ(25U, cut.getMaxJitter());
    }
    cut.reset();
    EXPECT_EQ(0U, cut.getTotalRuntime());
    EXPECT_EQ(0U, cut.getTotalRunCount());
    EXPECT_EQ(0U, cut.getMinRuntime());
    EXPECT_EQ(0U, cut.getMaxRuntime());
    EXPECT_EQ(0U, cut.getAverageRuntime());
    EXPECT_EQ(0U, cut.getMinJitter());
    EXPECT_EQ(0U, cut.getMaxJitter());
}

} // namespace
