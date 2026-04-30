// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   softwareSystemReset.cpp
 * \brief  Software-triggered NVIC system reset with pre-reset diagnostic hook.
 */

#include <mcu/mcu.h>
#include <reset/softwareSystemReset.h>

/**
 * \brief  Weak pre-reset diagnostic hook.
 *
 * Platforms can override this to flush CAN TX buffers or dump
 * diagnostic state before the reset takes effect.
 */
extern "C" __attribute__((weak)) void preResetDiag() {}

/**
 * \brief  Disable interrupts, invoke the diagnostic hook, and issue
 *         an NVIC system reset.
 *
 * \note   This function does not return.
 */
void softwareSystemReset()
{
    __disable_irq();
    preResetDiag();
    NVIC_SystemReset();
}
