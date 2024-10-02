// Copyright 2024 Accenture.

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
