// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#pragma once

// Required by the OpenBSW-patched core_cm4.h guard
#define INCLUDE_CORE_CM4_IN_MCU_H

// Include chip-specific CMSIS Device header (pulls in core_cm4.h internally)
#if defined(STM32F413xx)
#include "stm32f413xx.h"
#elif defined(STM32G474xx)
#include "stm32g474xx.h"
#else
#error "Unsupported STM32 device — define STM32F413xx or STM32G474xx"
#endif

#include "mcu/typedefs.h"

// Interrupt locking used by FreeRTOS CM4 SysTick port
#define ENABLE_INTERRUPTS()  __enable_irq()
#define DISABLE_INTERRUPTS() __disable_irq()

// STM32 uses 4-bit priority grouping (same as S32K1xx)
#define FEATURE_NVIC_PRIO_BITS (4U)
