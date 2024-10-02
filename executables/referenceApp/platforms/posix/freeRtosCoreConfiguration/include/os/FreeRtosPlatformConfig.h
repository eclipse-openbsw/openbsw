// Copyright 2024 Accenture.

#ifndef GUARD_40E410B8_9BD3_4B23_879D_FE0ABEB79F5B
#define GUARD_40E410B8_9BD3_4B23_879D_FE0ABEB79F5B

#ifdef __cplusplus
extern "C"
{
#endif

#define INCLUDE_uxTaskPriorityGet (1)
#undef configCHECK_FOR_STACK_OVERFLOW
#define configCHECK_FOR_STACK_OVERFLOW 2 // change from 1 in order to watermark check boundaries

// Disable legacy support for async

#ifndef ASYNC_LOOPER_DISABLE
#define ASYNC_LOOPER_DISABLE 1
#endif // ASYNC_LOOPER_DISABLE

#ifndef ASYNC_TIMEOUTMANAGER2_DISABLE
#define ASYNC_TIMEOUTMANAGER2_DISABLE 1
#endif // ASYNC_TIMEOUTMANAGER2_DISABLE

#ifndef MINIMUM_STACK_SIZE
#define EXTRA_THREAD_DATA_STACK_SIZE 40U // see pxPortInitialiseStack()
#if (defined(_DYNAMIC_STACK_SIZE_SOURCE)) || (defined(_GNU_SOURCE))
// the library may not give a compile time value; a workaround is provided
#define MINIMUM_STACK_SIZE (16384U + EXTRA_THREAD_DATA_STACK_SIZE)
#else // _DYNAMIC_STACK_SIZE_SOURCE
#include <pthread.h>
#define MINIMUM_STACK_SIZE ((PTHREAD_STACK_MIN) + EXTRA_THREAD_DATA_STACK_SIZE)
#endif // _DYNAMIC_STACK_SIZE_SOURCE
#endif // MINIMUM_STACK_SIZE

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* GUARD_40E410B8_9BD3_4B23_879D_FE0ABEB79F5B */
