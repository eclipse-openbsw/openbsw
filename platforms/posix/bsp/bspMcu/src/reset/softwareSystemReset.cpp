/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "reset/softwareSystemReset.h"

#include <unistd.h>

void terminal_cleanup(void);

extern "C"
{
[[noreturn]] void softwareSystemReset()
{
    terminal_cleanup();
    _exit(0);
}

void softwareDestructiveReset()
{
    terminal_cleanup();
    _exit(0);
}

} // extern "C"
