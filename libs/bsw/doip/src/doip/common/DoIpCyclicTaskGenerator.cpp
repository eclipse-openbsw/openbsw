/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/common/DoIpCyclicTaskGenerator.h"

#include <async/Async.h>

namespace doip
{
DoIpCyclicTaskGenerator::DoIpCyclicTaskGenerator(
    CyclicTaskType const cyclicTask,
    ::async::ContextType const context,
    uint16_t const cyclicTaskDelta)
: _cyclicTask(cyclicTask), _runTimeout(), _cyclicTaskDelta(cyclicTaskDelta), _context(context)
{}

void DoIpCyclicTaskGenerator::start()
{
    (void)::async::scheduleAtFixedRate(
        _context, *this, _runTimeout, _cyclicTaskDelta, ::async::TimeUnit::MILLISECONDS);
}

void DoIpCyclicTaskGenerator::shutdown() { (void)_runTimeout.cancel(); }

void DoIpCyclicTaskGenerator::execute() { _cyclicTask(); }

} // namespace doip
