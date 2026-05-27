/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <app/appConfig.h>

#include <platform/estdint.h>

// #include <portmacro.h>

extern uint32_t const __MPU_BSS_START[];
extern uint32_t const __MPU_BSS_END[];
extern ::async::internal::Stack<safety_task_stackSize> safetyStack;
