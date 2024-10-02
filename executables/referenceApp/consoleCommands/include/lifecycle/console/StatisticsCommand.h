// Copyright 2024 Accenture.

#ifndef GUARD_3426A8C5_0A38_4B74_BE74_D069EB452DF0
#define GUARD_3426A8C5_0A38_4B74_BE74_D069EB452DF0

#include <async/AsyncBinding.h>
#include <runtime/StatisticsContainer.h>
#include <util/command/GroupCommand.h>

#include <estd/optional.h>

namespace lifecycle
{
class StatisticsCommand : public ::util::command::GroupCommand
{
public:
    StatisticsCommand(::async::AsyncBinding::RuntimeMonitorType& runtimeMonitor);

    void setTicksPerUs(uint32_t ticksPerUs);
    void cyclic_1000ms();

protected:
    DECLARE_COMMAND_GROUP_GET_INFO
    void executeCommand(::util::command::CommandContext& context, uint8_t idx) override;

private:
    using TaskStatistics = ::runtime::declare::StatisticsContainer<
        ::runtime::RuntimeStatistics,
        ::async::AsyncBindingType::AdapterType::FREERTOS_TASK_COUNT>;

    using IsrGroupStatistics
        = ::runtime::declare::StatisticsContainer<::runtime::RuntimeStatistics, ISR_GROUP_COUNT>;

    ::async::AsyncBinding::RuntimeMonitorType& _runtimeMonitor;

    TaskStatistics _taskStatistics;
    IsrGroupStatistics _isrGroupStatistics;

    ::estd::optional<uint32_t> _ticksPerUs;
    uint32_t _totalRuntime;
};

} // namespace lifecycle

#endif /* GUARD_3426A8C5_0A38_4B74_BE74_D069EB452DF0 */
