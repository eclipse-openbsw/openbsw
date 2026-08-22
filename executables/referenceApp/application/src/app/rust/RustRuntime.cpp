/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "app/rust/RustRuntime.h"

#include <async/Async.h>
#include <async/Config.h>
#include <interrupts/suspendResumeAllInterrupts.h>

#include <cstdint>
#include <rust_runtime.h>

namespace
{
constexpr uint8_t MAX_TASKS             = static_cast<uint8_t>(ASYNC_CONFIG_TASK_COUNT);
::app::RustRuntime* runtimes[MAX_TASKS] = {nullptr};
} // namespace

namespace app
{
RustRuntime::RustRuntime(::async::ContextType const context) : _context(context) {}

void RustRuntime::init()
{
    if (_context < MAX_TASKS)
    {
        runtimes[_context] = this;
    }
    rust_runtime_init(_context);
}

void RustRuntime::schedule() { ::async::execute(_context, *this); }

void RustRuntime::execute() { rust_runtime_poll(_context); }

} // namespace app

extern "C"
{
void schedule_rust_runtime(uint8_t const task_id)
{
    if ((task_id < MAX_TASKS) && (runtimes[task_id] != nullptr))
    {
        runtimes[task_id]->schedule();
    }
}

uint32_t rust_critical_section_acquire(void)
{
    return getOldIntEnabledStatusValueAndSuspendAllInterrupts();
}

void rust_critical_section_release(uint32_t const restore_state)
{
    resumeAllInterrupts(restore_state);
}
}
