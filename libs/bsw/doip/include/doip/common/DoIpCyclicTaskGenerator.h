/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup doip
 */
#pragma once

#include <async/Types.h>
#include <etl/delegate.h>

namespace doip
{
/**
 * Class the implements a simple cyclic task generator.
 */
class DoIpCyclicTaskGenerator : private ::async::RunnableType
{
public:
    using CyclicTaskType = ::etl::delegate<void()>;

    /**
     * Constructor.
     * \param cyclicTask function that will be called periodically
     * \param context asynchronous execution context
     * \param cyclicTaskDelta task delta (in ms) of cyclic tasks
     */
    DoIpCyclicTaskGenerator(
        CyclicTaskType cyclicTask, ::async::ContextType context, uint16_t cyclicTaskDelta);

    /**
     * Start calling the cyclic task periodically.
     */
    void start();

    /**
     * Stop calling the cyclic tasks.
     */
    void shutdown();

private:
    void execute() override;

    CyclicTaskType _cyclicTask;
    ::async::TimeoutType _runTimeout;
    uint16_t _cyclicTaskDelta;
    ::async::ContextType _context;
};

} // namespace doip
