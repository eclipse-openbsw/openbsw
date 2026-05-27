/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "async/TickHookMock.h"

#include <etl/singleton_base.h>

namespace async
{
TickHookMock::TickHookMock() : ::etl::singleton_base<TickHookMock>(*this) {}

void TickHookMock::handleTick() { TickHookMock::instance().doHandleTick(); }

} // namespace async
