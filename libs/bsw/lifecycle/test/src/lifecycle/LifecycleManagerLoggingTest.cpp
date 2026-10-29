/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "lifecycle/LifecycleManager.h"

#include "lifecycle/LifecycleComponentMock.h"
#include "lifecycle/LifecycleListenerMock.h"
#include "lifecycle/LifecycleLogger.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <util/logger/ComponentMappingMock.h>
#include <util/logger/LoggerOutputMock.h>

#include <gtest/gtest.h>

#include <cstdio>
#include <string>
#include <utility>
#include <vector>

namespace
{
using namespace ::lifecycle;
using namespace ::testing;
using namespace ::util::logger;
using Transition = ILifecycleComponent::Transition::Type;
using Record     = std::pair<Level, std::string>;

class LifecycleManagerLoggingTest : public Test
{
protected:
    LifecycleManagerLoggingTest()
    {
        _context.handleAll();
        EXPECT_CALL(_mapping, isEnabled(LIFECYCLE, _))
            .WillRepeatedly(Invoke([this](uint8_t, Level level) { return level >= _threshold; }));
        EXPECT_CALL(_mapping, getComponentInfo(LIFECYCLE))
            .WillRepeatedly(Return(::util::logger::ComponentInfo(LIFECYCLE)));
        EXPECT_CALL(_mapping, getLevelInfo(_))
            .WillRepeatedly(Invoke(
                [](Level level)
                { return LevelInfo(&LevelInfo::getDefaultTable()[static_cast<size_t>(level)]); }));
        EXPECT_CALL(_output, logOutput(_, _, _, _))
            .WillRepeatedly(Invoke(
                [this](
                    ::util::logger::ComponentInfo const& component,
                    LevelInfo const& level,
                    char const* format,
                    va_list args)
                {
                    EXPECT_EQ(LIFECYCLE, component.getIndex());
                    char buffer[128];
                    int const size = vsnprintf(buffer, sizeof(buffer), format, args);
                    ASSERT_GE(size, 0);
                    ASSERT_LT(static_cast<size_t>(size), sizeof(buffer));
                    _records.emplace_back(level.getLevel(), buffer);
                }));
        Logger::init(_mapping, _output);
    }

    ~LifecycleManagerLoggingTest() override { Logger::shutdown(); }

    void addComponent(char const* name, LifecycleComponentMock& component, uint8_t level)
    {
        EXPECT_CALL(component, initCallback(_))
            .WillOnce(
                Invoke([this](ILifecycleComponentCallback& callback) { _callback = &callback; }));
        EXPECT_CALL(component, getTransitionContext(_))
            .WillRepeatedly(Return(::async::CONTEXT_INVALID));
        EXPECT_CALL(component, startTransition(_))
            .WillRepeatedly(
                Invoke([this, &component](Transition) { _callback->transitionDone(component); }));
        _manager.addComponent(name, component, level);
    }

    void transitionTo(uint8_t level)
    {
        _manager.transitionToLevel(level);
        _context.execute();
    }

    void checkPendingCompletion(Transition transition, uint8_t target, char const* completion)
    {
        _threshold = LEVEL_DEBUG;
        _records.clear();
        EXPECT_CALL(_first, startTransition(transition)).WillOnce(Return());
        EXPECT_CALL(_second, startTransition(transition)).WillOnce(Return());

        transitionTo(target);
        EXPECT_THAT(_records, Not(Contains(Pair(_, StrEq(completion)))));

        _callback->transitionDone(_second);
        _context.execute();
        EXPECT_THAT(_records, Not(Contains(Pair(_, StrEq(completion)))));

        _callback->transitionDone(_first);
        _context.execute();
        EXPECT_THAT(_records, Contains(Record(LEVEL_INFO, completion)).Times(1));

        _context.execute();
        EXPECT_THAT(_records, Contains(Record(LEVEL_INFO, completion)).Times(1));
    }

    static uint32_t timestamp() { return 0U; }

    StrictMock<::async::AsyncMock> _async;
    ::async::TestContext _context{1U};
    StrictMock<ComponentMappingMock> _mapping;
    StrictMock<LoggerOutputMock> _output;
    StrictMock<LifecycleComponentMock> _first;
    StrictMock<LifecycleComponentMock> _second;
    StrictMock<LifecycleListenerMock> _listener;
    Level _threshold = LEVEL_INFO;
    std::vector<Record> _records;
    ILifecycleComponentCallback* _callback = nullptr;
    ::lifecycle::declare::LifecycleManager<2, 3, 2> _manager{
        _context, LifecycleManager::GetTimestampType::create<&timestamp>()};
};

/**
 * INFO contains completed levels, not transition starts or component details.
 */
TEST_F(LifecycleManagerLoggingTest, InfoContainsOnlyCompletedLevels)
{
    addComponent("first", _first, 1U);

    transitionTo(1U);
    transitionTo(0U);

    EXPECT_THAT(
        _records,
        ElementsAre(
            Record(LEVEL_INFO, "Initialize level 1 done current=0 target=1"),
            Record(LEVEL_INFO, "Run level 1 done current=1 target=1"),
            Record(LEVEL_INFO, "Shutdown level 1 done current=0 target=0")));
}

/**
 * DEBUG retains level starts and both component starts and completions.
 */
TEST_F(LifecycleManagerLoggingTest, DebugIncludesDetailedTransitions)
{
    _threshold = LEVEL_DEBUG;
    addComponent("first", _first, 1U);

    transitionTo(1U);
    transitionTo(0U);

    EXPECT_THAT(
        _records,
        ElementsAre(
            Record(LEVEL_DEBUG, "Initialize level 1"),
            Record(LEVEL_DEBUG, "Initialize first"),
            Record(LEVEL_DEBUG, "Initialize first done"),
            Record(LEVEL_INFO, "Initialize level 1 done current=0 target=1"),
            Record(LEVEL_DEBUG, "Run level 1"),
            Record(LEVEL_DEBUG, "Run first"),
            Record(LEVEL_DEBUG, "Run first done"),
            Record(LEVEL_INFO, "Run level 1 done current=1 target=1"),
            Record(LEVEL_DEBUG, "Shutdown level 1"),
            Record(LEVEL_DEBUG, "Shutdown first"),
            Record(LEVEL_DEBUG, "Shutdown first done"),
            Record(LEVEL_INFO, "Shutdown level 1 done current=0 target=0")));
}

/**
 * INIT completion waits for every component and is not emitted twice.
 */
TEST_F(LifecycleManagerLoggingTest, InitCompletionWaitsForEveryComponent)
{
    addComponent("first", _first, 1U);
    addComponent("second", _second, 1U);
    checkPendingCompletion(Transition::INIT, 1U, "Initialize level 1 done current=0 target=1");
}

/**
 * RUN completion waits for every component and is not emitted twice.
 */
TEST_F(LifecycleManagerLoggingTest, RunCompletionWaitsForEveryComponent)
{
    addComponent("first", _first, 1U);
    addComponent("second", _second, 1U);
    checkPendingCompletion(Transition::RUN, 1U, "Run level 1 done current=1 target=1");
}

/**
 * SHUTDOWN completion waits for every component and is not emitted twice.
 */
TEST_F(LifecycleManagerLoggingTest, ShutdownCompletionWaitsForEveryComponent)
{
    addComponent("first", _first, 1U);
    addComponent("second", _second, 1U);
    transitionTo(1U);
    checkPendingCompletion(Transition::SHUTDOWN, 0U, "Shutdown level 1 done current=0 target=0");
}

/**
 * Empty intermediate level completions use INFO in either direction.
 */
TEST_F(LifecycleManagerLoggingTest, InfoIncludesEmptyIntermediateLevels)
{
    addComponent("first", _first, 1U);
    addComponent("second", _second, 3U);

    transitionTo(3U);
    transitionTo(0U);

    EXPECT_THAT(
        _records,
        ElementsAre(
            Record(LEVEL_INFO, "Initialize level 1 done current=0 target=3"),
            Record(LEVEL_INFO, "Run level 1 done current=1 target=3"),
            Record(LEVEL_INFO, "Initialize level 2 done current=1 target=3"),
            Record(LEVEL_INFO, "Run level 2 done current=2 target=3"),
            Record(LEVEL_INFO, "Initialize level 3 done current=2 target=3"),
            Record(LEVEL_INFO, "Run level 3 done current=3 target=3"),
            Record(LEVEL_INFO, "Shutdown level 3 done current=2 target=0"),
            Record(LEVEL_INFO, "Shutdown level 2 done current=1 target=0"),
            Record(LEVEL_INFO, "Shutdown level 1 done current=0 target=0")));
}

/**
 * Empty level starts use DEBUG while all completions remain INFO.
 */
TEST_F(LifecycleManagerLoggingTest, DebugIncludesEmptyLevelStartsAndInfoCompletions)
{
    _threshold = LEVEL_DEBUG;

    transitionTo(3U);
    transitionTo(0U);

    EXPECT_THAT(
        _records,
        ElementsAre(
            Record(LEVEL_DEBUG, "Initialize level 1"),
            Record(LEVEL_INFO, "Initialize level 1 done current=0 target=3"),
            Record(LEVEL_DEBUG, "Run level 1"),
            Record(LEVEL_INFO, "Run level 1 done current=1 target=3"),
            Record(LEVEL_DEBUG, "Initialize level 2"),
            Record(LEVEL_INFO, "Initialize level 2 done current=1 target=3"),
            Record(LEVEL_DEBUG, "Run level 2"),
            Record(LEVEL_INFO, "Run level 2 done current=2 target=3"),
            Record(LEVEL_DEBUG, "Initialize level 3"),
            Record(LEVEL_INFO, "Initialize level 3 done current=2 target=3"),
            Record(LEVEL_DEBUG, "Run level 3"),
            Record(LEVEL_INFO, "Run level 3 done current=3 target=3"),
            Record(LEVEL_DEBUG, "Shutdown level 3"),
            Record(LEVEL_INFO, "Shutdown level 3 done current=2 target=0"),
            Record(LEVEL_DEBUG, "Shutdown level 2"),
            Record(LEVEL_INFO, "Shutdown level 2 done current=1 target=0"),
            Record(LEVEL_DEBUG, "Shutdown level 1"),
            Record(LEVEL_INFO, "Shutdown level 1 done current=0 target=0")));
}

/**
 * Empty final RUN and SHUTDOWN transitions remain visible at INFO.
 */
TEST_F(LifecycleManagerLoggingTest, InfoIncludesEmptyFinalTargets)
{
    transitionTo(3U);
    transitionTo(0U);

    EXPECT_THAT(
        _records,
        ElementsAre(
            Record(LEVEL_INFO, "Initialize level 1 done current=0 target=3"),
            Record(LEVEL_INFO, "Run level 1 done current=1 target=3"),
            Record(LEVEL_INFO, "Initialize level 2 done current=1 target=3"),
            Record(LEVEL_INFO, "Run level 2 done current=2 target=3"),
            Record(LEVEL_INFO, "Initialize level 3 done current=2 target=3"),
            Record(LEVEL_INFO, "Run level 3 done current=3 target=3"),
            Record(LEVEL_INFO, "Shutdown level 3 done current=2 target=0"),
            Record(LEVEL_INFO, "Shutdown level 2 done current=1 target=0"),
            Record(LEVEL_INFO, "Shutdown level 1 done current=0 target=0")));
}

/**
 * The last nonempty startup level does not suppress a later empty target summary.
 */
TEST_F(LifecycleManagerLoggingTest, InfoIncludesEmptyRunTargetAfterComponentProgress)
{
    addComponent("first", _first, 1U);

    transitionTo(3U);

    EXPECT_THAT(
        _records,
        ElementsAre(
            Record(LEVEL_INFO, "Initialize level 1 done current=0 target=3"),
            Record(LEVEL_INFO, "Run level 1 done current=1 target=3"),
            Record(LEVEL_INFO, "Initialize level 2 done current=1 target=3"),
            Record(LEVEL_INFO, "Run level 2 done current=2 target=3"),
            Record(LEVEL_INFO, "Initialize level 3 done current=2 target=3"),
            Record(LEVEL_INFO, "Run level 3 done current=3 target=3")));
}

/**
 * Shutdown reports the completed transition level separately from current level.
 */
TEST_F(LifecycleManagerLoggingTest, InfoIncludesEmptyShutdownTargetAfterComponentProgress)
{
    addComponent("first", _first, 2U);
    transitionTo(2U);
    _records.clear();

    transitionTo(0U);

    EXPECT_THAT(
        _records,
        ElementsAre(
            Record(LEVEL_INFO, "Shutdown level 2 done current=1 target=0"),
            Record(LEVEL_INFO, "Shutdown level 1 done current=0 target=0")));
}

/**
 * INIT completion is not RUN completion and does not notify level listeners.
 */
TEST_F(LifecycleManagerLoggingTest, InitCompletionDoesNotNotifyRunListener)
{
    addComponent("first", _first, 1U);
    _manager.addLifecycleListener(_listener);
    EXPECT_CALL(_first, startTransition(Transition::RUN)).WillOnce(Return());

    transitionTo(1U);

    EXPECT_THAT(
        _records, ElementsAre(Record(LEVEL_INFO, "Initialize level 1 done current=0 target=1")));

    EXPECT_CALL(_listener, lifecycleLevelReached(1U, Transition::RUN));
    _callback->transitionDone(_first);
    _context.execute();
    EXPECT_THAT(_records, Contains(Record(LEVEL_INFO, "Run level 1 done current=1 target=1")));
}

/**
 * Every listener sees its completion record already emitted, including empty targets.
 */
TEST_F(LifecycleManagerLoggingTest, CompletionPrecedesListenersAndPreservesCallbackOrder)
{
    _manager.addLifecycleListener(_listener);
    InSequence const sequence;
    EXPECT_CALL(_listener, lifecycleLevelReached(1U, Transition::RUN))
        .WillOnce(Invoke(
            [this](uint8_t, Transition)
            {
                ASSERT_FALSE(_records.empty());
                EXPECT_EQ(
                    Record(LEVEL_INFO, "Run level 1 done current=1 target=1"), _records.back());
            }));
    EXPECT_CALL(_listener, lifecycleLevelReached(0U, Transition::SHUTDOWN))
        .WillOnce(Invoke(
            [this](uint8_t, Transition)
            {
                ASSERT_FALSE(_records.empty());
                EXPECT_EQ(
                    Record(LEVEL_INFO, "Shutdown level 1 done current=0 target=0"),
                    _records.back());
            }));

    transitionTo(1U);
    transitionTo(0U);
}

/**
 * Completion uses the live target after a request changes during a pending RUN.
 */
TEST_F(LifecycleManagerLoggingTest, CompletionReportsRetargetedLevel)
{
    addComponent("first", _first, 1U);
    EXPECT_CALL(_first, startTransition(Transition::RUN)).WillOnce(Return());
    transitionTo(1U);
    _records.clear();

    transitionTo(3U);
    EXPECT_TRUE(_records.empty());

    _callback->transitionDone(_first);
    _context.execute();

    EXPECT_THAT(
        _records,
        ElementsAre(
            Record(LEVEL_INFO, "Run level 1 done current=1 target=3"),
            Record(LEVEL_INFO, "Initialize level 2 done current=1 target=3"),
            Record(LEVEL_INFO, "Run level 2 done current=2 target=3"),
            Record(LEVEL_INFO, "Initialize level 3 done current=2 target=3"),
            Record(LEVEL_INFO, "Run level 3 done current=3 target=3")));
}

/**
 * A listener can retarget progress without changing the already emitted completion.
 */
TEST_F(LifecycleManagerLoggingTest, ListenerRetargetingPreservesProgression)
{
    _manager.addLifecycleListener(_listener);
    InSequence const sequence;
    EXPECT_CALL(_listener, lifecycleLevelReached(1U, Transition::RUN))
        .WillOnce(Invoke([this](uint8_t, Transition) { _manager.transitionToLevel(2U); }));
    EXPECT_CALL(_listener, lifecycleLevelReached(2U, Transition::RUN));

    transitionTo(1U);

    EXPECT_THAT(
        _records,
        ElementsAre(
            Record(LEVEL_INFO, "Initialize level 1 done current=0 target=1"),
            Record(LEVEL_INFO, "Run level 1 done current=1 target=1"),
            Record(LEVEL_INFO, "Initialize level 2 done current=1 target=2"),
            Record(LEVEL_INFO, "Run level 2 done current=2 target=2")));
}

} // namespace
