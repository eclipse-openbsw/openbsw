// Copyright 2024 Accenture.

#include "async/TickHookMock.h"

#include <etl/singleton_base.h>

namespace async
{
TickHookMock::TickHookMock() : ::etl::singleton_base<TickHookMock>(*this) {}

void TickHookMock::handleTick() { TickHookMock::instance().doHandleTick(); }

} // namespace async
