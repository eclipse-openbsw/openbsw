// Copyright 2024 Accenture.

#include "runtime/FunctionExecutionMonitor.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;
using namespace ::runtime;

struct TestStatistics
{
    uint32_t _value = 0U;

    void reset() { _value = 2U; }
};

struct TestEntry : public TestStatistics
{};

struct RuntimeMonitorMock
{
    using FunctionStatisticsType = TestStatistics;
    using FunctionEntryType      = TestEntry;

    MOCK_METHOD(void, enterFunction, (FunctionEntryType&));
    MOCK_METHOD(void, leaveFunction, (FunctionEntryType&));
};

using TestFunctionExecutionMonitorType = FunctionExecutionMonitor<RuntimeMonitorMock>;

struct FunctionExecutionMonitorTest : public Test
{
    StrictMock<RuntimeMonitorMock> _runtimeMonitorMock;
};

struct RuntimeMonitorMock2 : public RuntimeMonitorMock
{};

using TestFunctionExecutionMonitor2Type = FunctionExecutionMonitor<RuntimeMonitorMock2>;

TEST_F(FunctionExecutionMonitorTest, testPointSortOrder)
{
    {
        // initial list should be empty
        TestFunctionExecutionMonitorType::Iterator it;
        EXPECT_FALSE(it.hasValue());
    }
    {
        // add a point
        TestFunctionExecutionMonitorType::Point point1("point1");
        {
            TestFunctionExecutionMonitorType::Iterator it;
            EXPECT_TRUE(it.hasValue());
            EXPECT_STREQ("point1", it.getName());
            it.getStatistics();
            it.next();
            EXPECT_FALSE(it.hasValue());
        }
        {
            // add a second point. It should appear behind the first in the order
            TestFunctionExecutionMonitorType::Point point2("point2");
            {
                TestFunctionExecutionMonitorType::Iterator it;
                EXPECT_TRUE(it.hasValue());
                EXPECT_STREQ("point1", it.getName());
                it.next();
                EXPECT_TRUE(it.hasValue());
                EXPECT_STREQ("point2", it.getName());
                it.next();
                EXPECT_FALSE(it.hasValue());
            }
            {
                // add a third point. It should appear as the first
                TestFunctionExecutionMonitorType::Point point0("point0");
                {
                    TestFunctionExecutionMonitorType::Iterator it;
                    EXPECT_TRUE(it.hasValue());
                    EXPECT_STREQ("point0", it.getName());
                    it.next();
                    EXPECT_TRUE(it.hasValue());
                    EXPECT_STREQ("point1", it.getName());
                    it.next();
                    EXPECT_TRUE(it.hasValue());
                    EXPECT_STREQ("point2", it.getName());
                    it.next();
                    EXPECT_FALSE(it.hasValue());
                }
            }
            {
                // point 0 should be removed immediately
                TestFunctionExecutionMonitorType::Iterator it;
                EXPECT_TRUE(it.hasValue());
                EXPECT_STREQ("point1", it.getName());
                it.next();
                EXPECT_TRUE(it.hasValue());
                EXPECT_STREQ("point2", it.getName());
                it.next();
                EXPECT_FALSE(it.hasValue());
            }
        }
        {
            // point 2 should be removed immediately
            TestFunctionExecutionMonitorType::Iterator it;
            EXPECT_TRUE(it.hasValue());
            EXPECT_STREQ("point1", it.getName());
            it.getStatistics();
            it.next();
            EXPECT_FALSE(it.hasValue());
        }
    }
    {
        // initial list should be empty
        TestFunctionExecutionMonitorType::Iterator it;
        EXPECT_FALSE(it.hasValue());
    }
}

TEST_F(FunctionExecutionMonitorTest, testFunctionExecutionScope)
{
    TestFunctionExecutionMonitorType::init(_runtimeMonitorMock);
    TestFunctionExecutionMonitorType::Point point1("point1");
    TestFunctionExecutionMonitorType::Point point2("point2");
    {
        EXPECT_CALL(_runtimeMonitorMock, enterFunction(_));
        {
            TestFunctionExecutionMonitorType::Scope scope(point1);
            Mock::VerifyAndClearExpectations(&_runtimeMonitorMock);
            EXPECT_CALL(_runtimeMonitorMock, leaveFunction(_));
        }
        Mock::VerifyAndClearExpectations(&_runtimeMonitorMock);
    }
    {
        EXPECT_CALL(_runtimeMonitorMock, enterFunction(_));
        {
            TestFunctionExecutionMonitorType::Scope scope(point2);
            Mock::VerifyAndClearExpectations(&_runtimeMonitorMock);
            EXPECT_CALL(_runtimeMonitorMock, leaveFunction(_));
        }
        Mock::VerifyAndClearExpectations(&_runtimeMonitorMock);
    }
}

TEST_F(FunctionExecutionMonitorTest, testFunctionExecutionPointMacro)
{
    TestFunctionExecutionMonitorType::init(_runtimeMonitorMock);
    {
        EXPECT_CALL(_runtimeMonitorMock, enterFunction(_));
        FUNCTION_EXECUTION_POINT(RuntimeMonitorMock, "test");
        Mock::VerifyAndClearExpectations(&_runtimeMonitorMock);
        EXPECT_CALL(_runtimeMonitorMock, leaveFunction(_));
    }
    Mock::VerifyAndClearExpectations(&_runtimeMonitorMock);
}

TEST_F(FunctionExecutionMonitorTest, testTakeSnapshot)
{
    TestFunctionExecutionMonitorType::Point point("point1");
    EXPECT_EQ(0U, point._entry._value);
    EXPECT_EQ(0U, point._snapshot._value);
    point._entry._value = 1U;
    TestFunctionExecutionMonitorType::takeSnapshot();
    EXPECT_EQ(2U, point._entry._value);
    EXPECT_EQ(1U, point._snapshot._value);
}

TEST_F(FunctionExecutionMonitorTest, testIsUsed)
{
    {
        // A monitor that has used function points should be used
        EXPECT_TRUE(TestFunctionExecutionMonitorType::isUsed());
    }
    {
        // A monitor that has used function points should not be used
        EXPECT_FALSE(TestFunctionExecutionMonitor2Type::isUsed());
    }
}

} // namespace
