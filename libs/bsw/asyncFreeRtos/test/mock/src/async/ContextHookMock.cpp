/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "async/ContextHookMock.h"

#include <etl/singleton_base.h>

namespace async
{
ContextHookMock::ContextHookMock() : ::etl::singleton_base<ContextHookMock>(*this) {}

void ContextHookMock::enterTask(size_t taskIdx)
{
    ContextHookMock::instance().doEnterTask(taskIdx);
}

void ContextHookMock::leaveTask(size_t taskIdx)
{
    ContextHookMock::instance().doLeaveTask(taskIdx);
}

void ContextHookMock::enterIsrGroup(size_t isrGroupIdx)
{
    ContextHookMock::instance().doEnterIsrGroup(isrGroupIdx);
}

void ContextHookMock::leaveIsrGroup(size_t isrGroupIdx)
{
    ContextHookMock::instance().doLeaveIsrGroup(isrGroupIdx);
}

} // namespace async
