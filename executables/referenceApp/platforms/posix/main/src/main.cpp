// Copyright 2024 Accenture.

#include <async/AsyncBinding.h>
#include <lifecycle/LifecycleManager.h>

#include <estd/typed_mem.h>

#ifdef PLATFORM_SUPPORT_CAN
#include "systems/CanSystem.h"
#endif // PLATFORM_SUPPORT_CAN

extern void app_main();

namespace platform
{

#ifdef PLATFORM_SUPPORT_CAN
::estd::typed_mem<::systems::CanSystem> canSystem;
#endif // PLATFORM_SUPPORT_CAN

void platformLifecycleAdd(::lifecycle::LifecycleManager& lifecycleManager, uint8_t const level)
{
    if (level == 2)
    {
#ifdef PLATFORM_SUPPORT_CAN
        lifecycleManager.addComponent("can", canSystem.emplace(TASK_CAN), level);
#endif // PLATFORM_SUPPORT_CAN
    }
}

} // namespace platform

#ifdef PLATFORM_SUPPORT_CAN
namespace systems
{
::can::ICanSystem& getCanSystem() { return *::platform::canSystem; }
} // namespace systems
#endif // PLATFORM_SUPPORT_CAN

int main()
{
    app_main(); // entry point for the generic part
    return (1); // we never reach this point
}
