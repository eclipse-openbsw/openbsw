/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

// Required by the local core_cm4.h include guard.
#define INCLUDE_CORE_CM4_IN_MCU_H

#if defined(STM32F413xx)
#include "stm32f413xx.h"
#elif defined(STM32G474xx)
#include "stm32g474xx.h"
#else
#error "Unsupported STM32 device - define STM32F413xx or STM32G474xx"
#endif

#undef INCLUDE_CORE_CM4_IN_MCU_H
