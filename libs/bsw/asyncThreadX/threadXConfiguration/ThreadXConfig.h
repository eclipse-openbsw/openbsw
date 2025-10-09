// Copyright 2025 Accenture.

/**
 * \ingroup async
 */

/**
 * Async specific configuration
 *
 */

#pragma once

#include "async/Config.h"
#include "async/Hook.h"

/* Include the user defines in executables/referenceApp/platform/../tx_user.h. */

#define ASYNC_CONFIG_NESTED_INTERRUPTS (1)

#define ASYNC_TASK_CONFIG_TYPE void

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
void const* asyncGetTaskConfig(size_t taskIdx);

#ifdef __cplusplus
}
#endif // __cplusplus

#ifdef __cplusplus

namespace async
{
struct Config
{
    static size_t const TASK_COUNT = static_cast<size_t>(ASYNC_CONFIG_TASK_COUNT);
    static size_t const TICK_IN_US = static_cast<size_t>(ASYNC_CONFIG_TICK_IN_US);
};

} // namespace async

#endif // __cplusplus
