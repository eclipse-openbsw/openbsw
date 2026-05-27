/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "console/console.h"

#include <console/AsyncConsole.h>
#include <console/StdioConsoleInput.h>

namespace console
{
bool enableStdioConsole = false;
bool enableShmemConsole = false;

::console::StdioConsoleInput stdioConsoleInput(" ", "\r\n");
::console::AsyncConsole asyncConsole;

void enable() { enableStdioConsole = true; }

void disable() { enableStdioConsole = false; }

void init()
{
    stdioConsoleInput.init(
        StdioConsoleInput::OnLineReceived::create<AsyncConsole, &AsyncConsole::onLineReceived>(
            asyncConsole));
}

void run()
{
    if (enableStdioConsole)
    {
        stdioConsoleInput.run();
    }
}

} // namespace console
