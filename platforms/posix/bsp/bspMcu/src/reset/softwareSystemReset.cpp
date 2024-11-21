// Copyright 2024 Accenture.

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
