// Copyright 2024 Accenture.

#include "async/TickHookMock.h"

namespace async
{
TickHookMock::TickHookMock() : ::estd::singleton<TickHookMock>(*this) {}

void TickHookMock::handleTick() { TickHookMock::instance().doHandleTick(); }

} // namespace async
