// Copyright 2024 Accenture.

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

[[noreturn]] void softwareSystemReset() __attribute__((noreturn));
void softwareDestructiveReset() __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

