// Copyright 2024 Accenture.

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define INCLUDE_uxTaskPriorityGet (1)
#undef configCHECK_FOR_STACK_OVERFLOW
#define configCHECK_FOR_STACK_OVERFLOW 2 // change from 1 in order to watermark check boundaries

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
