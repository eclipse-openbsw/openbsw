/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "lifecycle/StaticBsp.h"

#include <async/AsyncBinding.h>
#include <etl/alignment.h>
#include <lifecycle/LifecycleManager.h>

#include <csignal>
#include <unistd.h>

extern void terminal_cleanup(void);
extern void main_thread_setup(void);

extern void app_main();

namespace platform
{
StaticBsp staticBsp;

StaticBsp& getStaticBsp() { return staticBsp; }

void platformLifecycleAdd(
    ::lifecycle::LifecycleManager& /* lifecycleManager */, uint8_t const /* level */)
{}

} // namespace platform

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
    ::platform::staticBsp.init();
    app_main(); // entry point for the generic part
    return (1); // we never reach this point
}
