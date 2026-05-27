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
#include "platform/estdint.h"

namespace uds
{
namespace UdsVmsConstants
{
static constexpr uint16_t MAX_BLOCK_LENGTH             = 4090U;
static constexpr uint16_t ERASE_MEMORY_TIME            = 120U;
static constexpr uint16_t CHECK_MEMORY_TIME            = 15U;
static constexpr uint16_t BOOTLOADER_INSTALLATION_TIME = 40U;
static constexpr uint16_t AUTHENTICATION_TIME          = 5U;
static constexpr uint16_t RESET_TIME                   = 10U;
static constexpr uint16_t TRANSFER_DATA_TIME           = 60U;

static constexpr uint32_t TESTER_PRESENT_TIMEOUT_MS = 5000U;

static constexpr uint8_t BUSY_MESSAGE_EXTRA_BYTES = 7U;
} /* namespace UdsVmsConstants */

namespace UdsConstants
{
/**
 * This is an extended timeout which is used when the programming
 * session in bootloader is entered. This is due to the fact that
 * a tester that is connected via ethernet may need more time
 * to reconnect (e.g. slow DHCP server)
 *
 * 60s no longer needed, changed to required 10s
 */
static constexpr uint32_t TESTER_PRESENT_EXTENDED_TIMEOUT = 10000U; // ms

/* Session Timeouts P2* and P2*/
/* Default */
static constexpr uint16_t DEFAULT_DIAG_RESPONSE_TIME        = 50U;  // ms
static constexpr uint16_t DEFAULT_DIAG_RESPONSE_PENDING     = 500U; // 10ms
/* Programming session */
static constexpr uint16_t PROGRAMMING_DIAG_RESPONSE_TIME    = 50U;   // ms
static constexpr uint16_t PROGRAMMING_DIAG_RESPONSE_PENDING = 5000U; // 10ms

/* Reset times */
/* ECUReset: HardReset -> 11 01 */
static constexpr uint16_t RESET_TIME_HARD = 1000U; // ms
/* ECUReset: SoftReset -> 11 03 */
static constexpr uint16_t RESET_TIME_SOFT = 1000U; // ms
/* ECUReset: EnableRapidPowerShutdown -> 11 04 */
static constexpr uint8_t SHUTDOWN_TIME    = 10U; // ms
/* DSC Reset */
static constexpr uint16_t RESET_TIME_DSC  = 1000U; // ms
} /* namespace UdsConstants */

} /* namespace uds */
