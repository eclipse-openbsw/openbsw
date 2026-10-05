/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <async/AsyncBinding.h>
#include <etl/optional.h>
#include <runtime/StatisticsContainer.h>
#include <util/command/GroupCommand.h>

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
        ::async::AsyncBindingType::AdapterType::OS_TASK_COUNT>;

    using IsrGroupStatistics
        = ::runtime::declare::StatisticsContainer<::runtime::RuntimeStatistics, ISR_GROUP_COUNT>;

    ::async::AsyncBinding::RuntimeMonitorType& _runtimeMonitor;

    TaskStatistics _taskStatistics;
    IsrGroupStatistics _isrGroupStatistics;

    ::etl::optional<uint32_t> _ticksPerUs;
    uint32_t _totalRuntime;
};

} // namespace lifecycle
