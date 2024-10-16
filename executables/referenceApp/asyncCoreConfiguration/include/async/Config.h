// Copyright 2024 Accenture.

#ifndef GUARD_BCC3EA4F_41A1_473E_9DA1_ADD5AE0FCC15
#define GUARD_BCC3EA4F_41A1_473E_9DA1_ADD5AE0FCC15

// NOTE: This file needs to be implemented in C as it ends up being part of FreeRTOSConfig.h
#define ASYNC_CONFIG_TICK_IN_US (100U) // System tick interval in microseconds.

/**
 * There are two implicit contexts/tasks: One is TASK_IDLE with the value 0 and the other is
 * TASK_TIMER with its value equal to ASYNC_CONFIG_TASK_COUNT. This implies that the actual number
 * of tasks in the system is not equal to ASYNC_CONFIG_TASK_COUNT but to ASYNC_CONFIG_TASK_COUNT+1.
 * And don't forget about the fact that we start counting at 1 here. To sum it up: If there are 3
 * entries in this enum, there are actually 5 tasks in the system.
 * See asyncFreeRtos/include/async/FreeRtosAdapter.h
 */

/**
 * Tasks for different operations are defined here.
 */
enum
{
    TASK_BACKGROUND = 1,
    TASK_BSP,
    TASK_UDS,
    TASK_DEMO,
    TASK_CAN,
    TASK_SYSADMIN,
    // --------------------
    ASYNC_CONFIG_TASK_COUNT,
};

/**
 * Groups for related interrupts are defined here.
 */
enum
{
    ISR_GROUP_TEST = 0,
    // ------------
    ISR_GROUP_CAN,
    ISR_GROUP_COUNT,
};

#endif /* GUARD_BCC3EA4F_41A1_473E_9DA1_ADD5AE0FCC15 */
