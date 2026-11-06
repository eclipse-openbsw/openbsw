/********************************************************************************
 * Copyright (c) 2024 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "reset/softwareSystemReset.h"

#include "hardware/watchdog.h"
#include "mcu/mcu.h"

#include <etl/infinite_loop.h>

extern "C"
{
[[noreturn]] void softwareSystemReset(void)
{
    watchdog_reboot(0, 0, 0);
    etl::infinite_loop();
}

void softwareDestructiveReset(void) { softwareSystemReset(); }
}
