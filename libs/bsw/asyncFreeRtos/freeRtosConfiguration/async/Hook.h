/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
