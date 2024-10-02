// Copyright 2024 Accenture.

#include "reset/softwareSystemReset.h"

#include <unistd.h>

extern "C"
{
[[noreturn]] void softwareSystemReset() { _exit(0); }

void softwareDestructiveReset() { _exit(0); }

} // extern "C"
