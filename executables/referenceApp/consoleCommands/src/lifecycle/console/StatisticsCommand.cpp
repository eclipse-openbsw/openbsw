// Copyright 2024 Accenture.

#include "lifecycle/console/StatisticsCommand.h"

#include <async/Async.h>
#include <runtime/StatisticsWriter.h>
#include <util/format/SharedStringWriter.h>

#include <estd/assert.h>

namespace
{
void format(
    ::runtime::StatisticsWriter& statisticsWriter, ::runtime::RuntimeStatistics const& statistics)
{
    statisticsWriter.writeRuntimePercentage("%", statistics.getTotalRuntime());
    statisticsWriter.writeRuntimeMS("total ", 9U, statistics.getTotalRuntime());
    statisticsWriter.writeNumber("runs ", 6U, statistics.getTotalRunCount());
    statisticsWriter.writeRuntime("avg ", 6U, statistics.getAverageRuntime());
    statisticsWriter.writeRuntime("min ", 6U, statistics.getMinRuntime());
    statisticsWriter.writeRuntime("max ", 6U, statistics.getMaxRuntime());
}

template<typename T, typename I>
void printCpu(
    ::util::command::CommandContext& context,
    T const& taskStatistics,
    I const& isrGroupStatistics,
    ::estd::optional<uint32_t> const& ticksPerUs,
    uint32_t const totalRuntime)
{
    ::util::format::SharedStringWriter writer(context);

    if (!ticksPerUs.has_value())
    {
        writer.printf("cannot print CPU statistics, ticksPerUs is unknown\n");
        return;
    }

    ::runtime::StatisticsWriter statisticsWriter(writer, totalRuntime, *ticksPerUs);

    typedef ::runtime::StatisticsWriter::FormatStatistics<::runtime::RuntimeStatistics>::Type
        FormatStatisticsType;

    FormatStatisticsType formatStatistics = FormatStatisticsType::create<&format>();

    statisticsWriter.formatStatisticsGroup(
        formatStatistics, "task", 15U, taskStatistics.getIterator());

    statisticsWriter.writeEol();

    statisticsWriter.formatStatisticsGroup(
        formatStatistics, "isr group", 15U, isrGroupStatistics.getIterator());

    statisticsWriter.writeEol();

    writer.write("measurement time: ");

    statisticsWriter.writeRuntime("", 8U, totalRuntime);
}

void printStack(
    ::util::command::CommandContext& context,
    ::async::AsyncBinding::RuntimeMonitorType& /* runtimeMonitor */)
{
    ::util::format::SharedStringWriter writer(context);

    for (size_t i = 0; i <= ASYNC_CONFIG_TASK_COUNT; ++i)
    {
        using at               = ::async::AsyncBindingType::AdapterType;
        char const* const name = at::getTaskName(i);
        at::StackUsage stackUsage;
        at::getStackUsage(i, stackUsage);

        writer.printf(
            "stack:task=%s,size=%d,used=%d\n", name, stackUsage._stackSize, stackUsage._usedSize);
    }
}

enum Id
{
    ID_CPU,
    ID_STACK,
    ID_ALL
};

} // namespace

namespace lifecycle
{
DEFINE_COMMAND_GROUP_GET_INFO_BEGIN(StatisticsCommand, "stats", "lifecycle statistics command")
COMMAND_GROUP_COMMAND(ID_CPU, "cpu", "prints CPU statistics")
COMMAND_GROUP_COMMAND(ID_STACK, "stack", "prints stack statistics")
COMMAND_GROUP_COMMAND(ID_ALL, "all", "prints all statistics")
DEFINE_COMMAND_GROUP_GET_INFO_END

StatisticsCommand::StatisticsCommand(::async::AsyncBinding::RuntimeMonitorType& runtimeMonitor)
: _runtimeMonitor(runtimeMonitor)
, _taskStatistics()
, _isrGroupStatistics()
, _ticksPerUs()
, _totalRuntime(0)
{}

void StatisticsCommand::setTicksPerUs(uint32_t const ticksPerUs) { _ticksPerUs = ticksPerUs; }

void StatisticsCommand::cyclic_1000ms()
{
    ::async::Lock const lock;
    _taskStatistics.copyFrom(_runtimeMonitor.getTaskStatistics());
    _isrGroupStatistics.copyFrom(_runtimeMonitor.getIsrGroupStatistics());
    _totalRuntime = _runtimeMonitor.reset();
}

void StatisticsCommand::executeCommand(::util::command::CommandContext& context, uint8_t idx)
{
    switch (idx)
    {
        case ID_CPU:
        {
            printCpu(context, _taskStatistics, _isrGroupStatistics, _ticksPerUs, _totalRuntime);
            break;
        }
        case ID_STACK:
        {
            printStack(context, _runtimeMonitor);
            break;
        }
        case ID_ALL:
        {
            printCpu(context, _taskStatistics, _isrGroupStatistics, _ticksPerUs, _totalRuntime);
            printStack(context, _runtimeMonitor);
            break;
        }
        default:
        {
            break;
        }
    }
}

} // namespace lifecycle
