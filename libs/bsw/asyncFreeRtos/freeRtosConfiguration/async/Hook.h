// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stddef.h>
#include <stdint.h>

void asyncEnterTask(size_t taskIdx);
void asyncLeaveTask(size_t taskIdx);
void asyncEnterIsrGroup(size_t isrGroupIdx);
void asyncLeaveIsrGroup(size_t isrGroupIdx);
uint32_t asyncTickHook(void);

#ifdef __cplusplus
}
#endif // __cplusplus
