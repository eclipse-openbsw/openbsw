// Copyright 2024 Accenture.

#include "runtime/RuntimeMonitor.h"

#include "bsp/timer/SystemTimerMock.h"

namespace
{
using namespace ::testing;
using namespace ::runtime;

class TestContextStatistics
{
public:
    MOCK_METHOD(void, addRun, (uint32_t, uint32_t, uint32_t));
    MOCK_METHOD(void, reset, ());
};

class TestFunctionStatistics
{
public:
    MOCK_METHOD(void, addRun, (uint32_t, uint32_t, uint32_t));
};

class RuntimeMonitorTest : public Test
{
public:
    MOCK_METHOD(char const*, getTaskName, (size_t));

protected:
    StrictMock<SystemTimerMock> _systemTimerMock;
};

TEST_F(RuntimeMonitorTest, testAll)
{
    typedef RuntimeMonitor<TestContextStatistics, TestFunctionStatistics> CutType;
    CutType::ContextEntryType taskEntries[3];
    CutType::ContextStatisticsContainerType taskContainer(
        taskEntries,
        CutType::ContextStatisticsContainerType::GetNameType::
            create<RuntimeMonitorTest, &RuntimeMonitorTest::getTaskName>(*this));
    CutType::ContextEntryType isrGroupEntries[2];
    CutType::ContextStatisticsContainerType isrGroupContainer(
        isrGroupEntries,
        CutType::ContextStatisticsContainerType::GetNameType::
            create<RuntimeMonitorTest, &RuntimeMonitorTest::getTaskName>(*this));
    CutType::FunctionEntryType functionEntries[2];
    CutType cut(taskContainer, isrGroupContainer);
    {
        // check containers
        EXPECT_EQ(&taskContainer, &cut.getTaskStatistics());
        EXPECT_EQ(&isrGroupContainer, &cut.getIsrGroupStatistics());
    }
    {
        // start the monitor
        EXPECT_CALL(_systemTimerMock, getSystemTicks32Bit()).WillOnce(Return(1000U));
        cut.start();
        Mock::VerifyAndClearExpectations(&_systemTimerMock);
    }
    {
        // no function can be added before first task
        cut.enterFunction(functionEntries[1U]);
        cut.leaveFunction(functionEntries[1U]);
        Mock::VerifyAndClearExpectations(&_systemTimerMock);
    }
    {
        // enter task
        EXPECT_CALL(_systemTimerMock, getSystemTicks32Bit()).WillOnce(Return(1010U));
        cut.enterTask(0U);
        Mock::VerifyAndClearExpectations(&_systemTimerMock);
    }
    {
        // enter function
        EXPECT_CALL(_systemTimerMock, getSystemTicks32Bit()).WillOnce(Return(1020U));
        cut.enterFunction(functionEntries[0U]);
        Mock::VerifyAndClearExpectations(&_systemTimerMock);
    }
    {
        // leave current task => expect run to be added
        EXPECT_CALL(_systemTimerMock, getSystemTicks32Bit()).WillOnce(Return(1038U));
        EXPECT_CALL(taskEntries[0], addRun(1010U, 28U, 0U));
        cut.leaveTask(0U);
        Mock::VerifyAndClearExpectations(&_systemTimerMock);
    }
    {
        // enter next task
        EXPECT_CALL(_systemTimerMock, getSystemTicks32Bit()).WillOnce(Return(1040U));
        cut.enterTask(1U);
        Mock::VerifyAndClearExpectations(&_systemTimerMock);
    }
    {
        // enter previous task
        EXPECT_CALL(_systemTimerMock, getSystemTicks32Bit()).WillOnce(Return(1089U));
        EXPECT_CALL(taskEntries[1], addRun(1040U, 49U, 0U));
        cut.leaveTask(1U);
        Mock::VerifyAndClearExpectations(&_systemTimerMock);
    }
    {
        EXPECT_CALL(_systemTimerMock, getSystemTicks32Bit()).WillOnce(Return(1090U));
        cut.enterTask(0U);
        Mock::VerifyAndClearExpectations(&_systemTimerMock);
    }
    {
        // reset and expect time difference to previous start to be returned
        EXPECT_CALL(taskEntries[0U], reset());
        EXPECT_CALL(taskEntries[1U], reset());
        EXPECT_CALL(taskEntries[2U], reset());
        EXPECT_CALL(isrGroupEntries[0U], reset());
        EXPECT_CALL(isrGroupEntries[1U], reset());
        EXPECT_EQ(90U, cut.reset());
    }
    {
        // enter interrupt context
        EXPECT_CALL(_systemTimerMock, getSystemTicks32Bit()).WillOnce(Return(1190U));
        cut.enterIsrGroup(0U);
        Mock::VerifyAndClearExpectations(&_systemTimerMock);
    }
    {
        // enter nested interrupt context
        EXPECT_CALL(_systemTimerMock, getSystemTicks32Bit()).WillOnce(Return(1230U));
        cut.enterIsrGroup(1U);
        Mock::VerifyAndClearExpectations(&_systemTimerMock);
    }
    {
        // leave nested interrupt context
        EXPECT_CALL(_systemTimerMock, getSystemTicks32Bit()).WillOnce(Return(1300U));
        EXPECT_CALL(isrGroupEntries[1U], addRun(1230U, 70U, 0U));
        cut.leaveIsrGroup(1);
        Mock::VerifyAndClearExpectations(&_systemTimerMock);
    }
    {
        // leave interrupt context
        EXPECT_CALL(_systemTimerMock, getSystemTicks32Bit()).WillOnce(Return(1450U));
        EXPECT_CALL(isrGroupEntries[0U], addRun(1190U, 190U, 70U));
        cut.leaveIsrGroup(0);
        Mock::VerifyAndClearExpectations(&_systemTimerMock);
    }
    {
        // enter nested function
        EXPECT_CALL(_systemTimerMock, getSystemTicks32Bit()).WillOnce(Return(1500U));
        cut.enterFunction(functionEntries[1U]);
        Mock::VerifyAndClearExpectations(&_systemTimerMock);
    }
    {
        // leave nested function
        EXPECT_CALL(_systemTimerMock, getSystemTicks32Bit()).WillOnce(Return(1600U));
        EXPECT_CALL(functionEntries[1U], addRun(1500U, 100U, 0U));
        cut.leaveFunction(functionEntries[1U]);
        Mock::VerifyAndClearExpectations(&_systemTimerMock);
    }
    {
        // leave function
        EXPECT_CALL(_systemTimerMock, getSystemTicks32Bit()).WillOnce(Return(1750U));
        EXPECT_CALL(functionEntries[0U], addRun(1020U, 418U, 312U));
        cut.leaveFunction(functionEntries[0U]);
        Mock::VerifyAndClearExpectations(&_systemTimerMock);
    }
    {
        // stop the monitor
        EXPECT_CALL(_systemTimerMock, getSystemTicks32Bit()).WillOnce(Return(1850U));
        EXPECT_CALL(taskEntries[0U], addRun(1090U, 500U, 260U));
        cut.stop();
        Mock::VerifyAndClearExpectations(&_systemTimerMock);
    }
}

TEST_F(RuntimeMonitorTest, testDeclare)
{
    typedef ::runtime::declare::
        RuntimeMonitor<TestContextStatistics, TestFunctionStatistics, 2U, 3U>
            CutType;
    char const* const isrGroupNames[] = {"group1", "group2", "group3"};
    CutType cut(
        CutType::ContextStatisticsContainerType::GetNameType::
            create<RuntimeMonitorTest, &RuntimeMonitorTest::getTaskName>(*this),
        isrGroupNames);
    {
        char const* name = "task";
        EXPECT_EQ(2U, cut.getTaskStatistics().getSize());
        EXPECT_CALL(*this, getTaskName(1U)).WillOnce(Return(name));
        EXPECT_EQ(name, cut.getTaskStatistics().getName(1U));
    }
    {
        EXPECT_EQ(3U, cut.getIsrGroupStatistics().getSize());
        EXPECT_EQ(isrGroupNames[1], cut.getIsrGroupStatistics().getName(1U));
    }
}

} // namespace
