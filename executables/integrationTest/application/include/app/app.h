/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "lifecycle/StaticBsp.h"
#include <lifecycle/LifecycleManager.h>

namespace app
{
void run();
}

namespace platform
{
extern void platformLifecycleAdd(::lifecycle::LifecycleManager& lifecycleManager, uint8_t level);
extern StaticBsp& getStaticBsp();
} // namespace platform
