// Copyright 2024 Accenture.

#pragma once

#include "3rdparty/nxp/S32K148.h"
#if defined(__MPU_PRESENT) && (__MPU_PRESENT == 1U)
// NXP has their own MPU, so we must not use the cmsis one.
#undef __MPU_PRESENT
#define INCLUDE_CORE_CM4_IN_MCU_H
#include "3rdparty/cmsis/core_cm4.h"
#undef INCLUDE_CORE_CM4_IN_MCU_H
#define __MPU_PRESENT 1
#else
#include "mcu/core_cm4.h"
#endif /* defined (__MPU_PRESENT) && (__MPU_PRESENT == 1U) */
#include "mcu/typedefs.h"

// interrupt locking and nvic info used by freertos cm4 systick
#define ENABLE_INTERRUPTS()  __enable_irq()
#define DISABLE_INTERRUPTS() __disable_irq()

#define FEATURE_NVIC_PRIO_BITS (4U)

// watchdog magic values
#define FEATURE_WDOG_UNLOCK_VALUE  (0xD928C520U)
#define FEATURE_WDOG_TRIGGER_VALUE (0xB480A602U)

// FIRCERR bit mask not defined in S32K148 header file
#define SCG_FIRCCSR_FIRCERR_MASK 0x4000000u

// IP/device remapping, extend as needed
#define LMEM           IP_LMEM
#define WDOG           IP_WDOG
#define LPUART0        IP_LPUART0
#define LPUART1        IP_LPUART1
#define LPIT0          IP_LPIT0
#define ADC0           IP_ADC0
#define FTFC           IP_FTFC
#define FTM0           IP_FTM0
#define FTM1           IP_FTM1
#define FTM2           IP_FTM2
#define FTM3           IP_FTM3
#define FTM4           IP_FTM4
#define SIM            IP_SIM
#define RCM            IP_RCM
#define SCG            IP_SCG
#define PCC            IP_PCC
#define ERM            IP_ERM
#define FTFC           IP_FTFC
#define CRC            IP_CRC
#define GPIO_BASE_PTRS IP_GPIO_BASE_PTRS
#define PORT_BASE_PTRS IP_PORT_BASE_PTRS
