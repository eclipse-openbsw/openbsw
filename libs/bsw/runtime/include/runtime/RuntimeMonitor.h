// Copyright 2024 Accenture.

/**
 * \ingroup runtime
 */
#pragma once

#include "async/Types.h"
#include "bsp/timer/SystemTimer.h"
#include "runtime/NestedRuntimeEntry.h"
#include "runtime/RuntimeStack.h"
#include "runtime/SimpleRuntimeEntry.h"
#include "runtime/StatisticsContainer.h"

#include <estd/slice.h>

#include <cstdint>

namespace runtime
{
template<class ContextStatistics, class FunctionStatistics>
class RuntimeMonitor
{
public:
    using ContextStatisticsType  = ContextStatistics;
    using FunctionStatisticsType = FunctionStatistics;

    class FunctionEntry : public SimpleRuntimeEntry<FunctionEntry, FunctionStatistics, false>
    {};

    using FunctionEntryType = FunctionEntry;

    class ContextEntry
    : public NestedRuntimeEntry<ContextEntry, ContextStatistics, true, FunctionEntryType>
    {};

    using ContextEntryType = ContextEntry;

    using ContextStatisticsContainerType = StatisticsContainer<ContextStatistics, ContextEntryType>;

    RuntimeMonitor(
        ContextStatisticsContainerType& taskStatistics,
        ContextStatisticsContainerType& isrGroupStatistics)
    : _taskStatistics(taskStatistics)
    , _isrGroupStatistics(isrGroupStatistics)
    , _startTimestamp(0U)
    , _lastEnterTaskTimestamp(0U)
    {}

    ContextStatisticsContainerType const& getTaskStatistics() const { return _taskStatistics; }

    ContextStatisticsContainerType const& getIsrGroupStatistics() const
    {
        return _isrGroupStatistics;
    }

    uint32_t getLastRuntime()
    {
        ::async::LockType const lock;
        return (_lastEnterTaskTimestamp - _startTimestamp);
    }

    void start()
    {
        _startTimestamp         = getSystemTicks32Bit();
        _lastEnterTaskTimestamp = _startTimestamp;
    }

    void stop()
    {
        ::async::LockType const lock;
        _contextStack.popEntry(getSystemTicks32Bit());
    }

    uint32_t reset()
    {
        ::async::LockType const lock;
        uint32_t const lastRuntime = _lastEnterTaskTimestamp - _startTimestamp;
        _startTimestamp            = _lastEnterTaskTimestamp;
        _taskStatistics.reset();
        _isrGroupStatistics.reset();
        return lastRuntime;
    }

    static void resetContextEntries(::estd::slice<ContextEntryType> contextEntries);

    void enterTask(size_t const taskIdx)
    {
        ::async::LockType const lock;
        uint32_t const timestamp = getSystemTicks32Bit();
        _lastEnterTaskTimestamp  = timestamp;
        _contextStack.pushEntry(_taskStatistics.getEntry(taskIdx), timestamp);
    }

    void leaveTask(size_t const taskIdx)
    {
        ::async::LockType const lock;
        uint32_t const timestamp = getSystemTicks32Bit();
        _contextStack.popEntry(_taskStatistics.getEntry(taskIdx), timestamp);
    }

    void enterIsrGroup(size_t const isrGroupIdx)
    {
        ::async::LockType const lock;
        uint32_t const timestamp = getSystemTicks32Bit();
        _contextStack.pushEntry(_isrGroupStatistics.getEntry(isrGroupIdx), timestamp);
    }

    void leaveIsrGroup(size_t const isrGroupIdx)
    {
        ::async::LockType const lock;
        uint32_t const timestamp = getSystemTicks32Bit();
        _contextStack.popEntry(_isrGroupStatistics.getEntry(isrGroupIdx), timestamp);
    }

    void enterFunction(FunctionEntryType& functionEntry) const
    {
        ::async::LockType const lock;
        ContextEntryType* const topEntry = _contextStack.getTopEntry();
        if (topEntry != nullptr)
        {
            topEntry->pushEntry(functionEntry, getSystemTicks32Bit());
        }
    }

    void leaveFunction(FunctionEntryType& functionEntry) const
    {
        ::async::LockType const lock;
        ContextEntryType* const topEntry = _contextStack.getTopEntry();
        if (topEntry != nullptr)
        {
            topEntry->popEntry(functionEntry, getSystemTicks32Bit());
        }
    }

private:
    RuntimeStack<ContextEntryType> _contextStack{};
    ContextStatisticsContainerType& _taskStatistics;
    ContextStatisticsContainerType& _isrGroupStatistics;
    uint32_t _startTimestamp;
    uint32_t _lastEnterTaskTimestamp;
};

namespace declare
{
template<
    class ContextStatistics,
    class FunctionStatistics,
    size_t TASK_COUNT,
    size_t ISR_GROUP_COUNT>
class RuntimeMonitor : public ::runtime::RuntimeMonitor<ContextStatistics, FunctionStatistics>
{
public:
    using ContextEntryType =
        typename ::runtime::RuntimeMonitor<ContextStatistics, FunctionStatistics>::ContextEntryType;
    using GetNameType = typename ::runtime::RuntimeMonitor<ContextStatistics, FunctionStatistics>::
        ContextStatisticsContainerType::GetNameType;

    RuntimeMonitor(
        GetNameType const getTaskName,
        ::estd::slice<char const* const, ISR_GROUP_COUNT> const& isrGroupNames)
    : ::runtime::RuntimeMonitor<ContextStatistics, FunctionStatistics>(
        _taskStatistics, _isrGroupStatistics)
    , _taskStatistics(getTaskName)
    , _isrGroupStatistics(
          GetNameType::template create<RuntimeMonitor, &RuntimeMonitor::getIsrGroupName>(*this))
    , _isrGroupNames(isrGroupNames)
    {}

private:
    char const* getIsrGroupName(size_t const isrGroupName) const
    {
        return _isrGroupNames[isrGroupName];
    }

    ::runtime::declare::StatisticsContainer<ContextStatistics, TASK_COUNT, ContextEntryType>
        _taskStatistics;
    ::runtime::declare::StatisticsContainer<ContextStatistics, ISR_GROUP_COUNT, ContextEntryType>
        _isrGroupStatistics;
    ::estd::slice<char const* const> _isrGroupNames;
};

} // namespace declare
} // namespace runtime
