// Copyright 2024 Accenture.

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
