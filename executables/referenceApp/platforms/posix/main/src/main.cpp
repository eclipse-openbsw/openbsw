// Copyright 2024 Accenture.

#include "lifecycle/StaticBsp.h"

#include <async/AsyncBinding.h>
#include <etl/alignment.h>
#include <lifecycle/LifecycleManager.h>

#include <signal.h>
#include <unistd.h>

#ifdef PLATFORM_SUPPORT_CAN
#include "systems/CanSystem.h"
#endif // PLATFORM_SUPPORT_CAN

extern void terminal_setup(void);
extern void terminal_cleanup(void);
extern void main_thread_setup(void);
#ifdef PLATFORM_SUPPORT_ETHERNET
#include "systems/TapEthernetSystem.h"
#endif // PLATFORM_SUPPORT_ETHERNET

extern void app_main();

namespace platform
{
StaticBsp staticBsp;

#ifdef PLATFORM_SUPPORT_CAN
::etl::typed_storage<::systems::CanSystem> canSystem;
#endif // PLATFORM_SUPPORT_CAN

#ifdef PLATFORM_SUPPORT_ETHERNET
::etl::typed_storage<::systems::TapEthernetSystem> tapEthernetSystem;
#endif // PLATFORM_SUPPORT_ETHERNET

void platformLifecycleAdd(::lifecycle::LifecycleManager& lifecycleManager, uint8_t const level)
{
    if (level == 2)
    {
#ifdef PLATFORM_SUPPORT_CAN
        lifecycleManager.addComponent("can", canSystem.create(TASK_CAN), level);
#endif // PLATFORM_SUPPORT_CAN
#ifdef PLATFORM_SUPPORT_ETHERNET
        lifecycleManager.addComponent("eth", tapEthernetSystem.create(TASK_ETHERNET), level);
#endif // PLATFORM_SUPPORT_ETHERNET
    }
}

} // namespace platform

#ifdef PLATFORM_SUPPORT_CAN
namespace systems
{
::can::ICanSystem& getCanSystem() { return *::platform::canSystem; }
} // namespace systems
#endif // PLATFORM_SUPPORT_CAN

#ifdef PLATFORM_SUPPORT_ETHERNET
namespace systems
{
::ethernet::IEthernetDriverSystem& getEthernetSystem() { return *::platform::tapEthernetSystem; }
} // namespace systems
#endif // PLATFORM_SUPPORT_ETHERNET

extern "C"
{
void putchar_(char character) { putchar(character); }
}

void intHandler(int /* sig */)
{
    terminal_cleanup();
    _exit(0);
}

int main()
{
    signal(SIGINT, intHandler);
    main_thread_setup();
    terminal_setup();
    ::platform::staticBsp.init();
    app_main(); // entry point for the generic part
    return (1); // we never reach this point
}
