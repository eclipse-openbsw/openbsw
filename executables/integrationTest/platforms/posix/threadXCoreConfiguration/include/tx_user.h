// Copyright 2025 Accenture.

#pragma once

/* Override various options with default values already assigned in tx_port.h. Please also refer
   to tx_port.h for descriptions on each of these options.  */

#include "async/Config.h"

#define ROUND_UP_32(x)          (((x) + 31U) & ~31U)
#define TX_MAX_PRIORITIES_LIMIT 1024
#define TX_MIN_PRIORITIES       32

/* Define the priority levels for ThreadX.  Legal values range
   from 32 to 1024 and MUST be evenly divisible by 32.  */
#ifdef TX_MAX_PRIORITIES
#undef TX_MAX_PRIORITIES
#endif

#define TX_MAX_PRIORITIES                                                   \
    ((ROUND_UP_32(ASYNC_CONFIG_TASK_COUNT + 1U)) > TX_MAX_PRIORITIES_LIMIT  \
         ? TX_MAX_PRIORITIES_LIMIT                                          \
         : ((ROUND_UP_32(ASYNC_CONFIG_TASK_COUNT + 1U)) < TX_MIN_PRIORITIES \
                ? TX_MIN_PRIORITIES                                         \
                : ROUND_UP_32(ASYNC_CONFIG_TASK_COUNT + 1U)))

#if (TX_MAX_PRIORITIES < 32) || (TX_MAX_PRIORITIES > 1024) || ((TX_MAX_PRIORITIES % 32) != 0)
#error "TX_MAX_PRIORITIES must be between 32 and 1024 and evenly divisible by 32"
#endif

/* Define the minimum stack for a ThreadX thread on this processor. If the size supplied during
   thread creation is less than this value, the thread create call will return an error.  */
#ifndef TX_MINIMUM_STACK
#define TX_MINIMUM_STACK (1024U)
#endif

// #define TX_LINUX_NO_IDLE_ENABLE
#define TX_TIMER_TICKS_PER_SECOND \
    (1000000U / ASYNC_CONFIG_TICK_IN_US) // System tick interval in seconds.

#define TX_NO_FILEX_POINTER

#define TX_DISABLE_PREEMPTION_THRESHOLD

#ifndef TX_MAX_PRIORITIES
#define TX_MAX_PRIORITIES
#endif

#define TX_LINUX_MEMORY_SIZE (256000U)

#ifndef TX_LINUX_MULTI_CORE
#define TX_LINUX_MULTI_CORE
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifdef __linux__
#include <sched.h>
#endif
