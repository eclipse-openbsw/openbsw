/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "async/Hook.h"

#include "async/AsyncBinding.h"

namespace
{
using AdapterType     = ::async::AsyncBindingType::AdapterType;
using ContextHookType = ::async::AsyncBindingType::ContextHookType;
} // namespace

extern "C"
{
void const* asyncGetTaskConfig(size_t const taskIdx) { return AdapterType::getTaskConfig(taskIdx); }

void asyncEnterTask(size_t const taskIdx) { ContextHookType::enterTask(taskIdx); }

void asyncLeaveTask(size_t const taskIdx) { ContextHookType::leaveTask(taskIdx); }

void asyncEnterIsrGroup(size_t const isrGroupIdx) { ContextHookType::enterIsrGroup(isrGroupIdx); }

void asyncLeaveIsrGroup(size_t const isrGroupIdx) { ContextHookType::leaveIsrGroup(isrGroupIdx); }

#if ASYNC_CONFIG_TICK_HOOK
uint32_t asyncTickHook()
{
    ::async::AsyncBindingType::TickHookType::handleTick();
    return 1U;
}
#endif // ASYNC_CONFIG_TICK_HOOK

} // extern "C"
