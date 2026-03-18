// Copyright 2025 Accenture.

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
