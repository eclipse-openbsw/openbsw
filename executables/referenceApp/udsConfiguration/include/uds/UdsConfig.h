// Copyright 2024 Accenture.
// Copyright 2026 BMW AG

#pragma once
#include "platform/estdint.h"

namespace uds
{
class UdsVmsConstants
{
public:
    static uint16_t const MAX_BLOCK_LENGTH             = 4090U;
    static uint16_t const ERASE_MEMORY_TIME            = 120U;
    static uint16_t const CHECK_MEMORY_TIME            = 15U;
    static uint16_t const BOOTLOADER_INSTALLATION_TIME = 40U;
    static uint16_t const AUTHENTICATION_TIME          = 5U;
    static uint16_t const RESET_TIME                   = 10U;
    static uint16_t const TRANSFER_DATA_TIME           = 60U;
    static uint32_t const TESTER_PRESENT_TIMEOUT_MS    = 5000U;
    static uint8_t const BUSY_MESSAGE_EXTRA_BYTES      = 7U;
};

class UdsConstants
{
public:
    /**
     * This is an extended timeout which is used when the programming
     * session in bootloader is entered. This is due to the fact that
     * a tester that is connected via ethernet may need more time
     * to reconnect (e.g. slow DHCP server)
     *
     * 60s no longer needed, changed to required 10s
     */
    static uint32_t const TESTER_PRESENT_EXTENDED_TIMEOUT = 10000U; // ms

    /* Session Timeouts P2* and P2*/
    /* Default */
    static uint16_t const DEFAULT_DIAG_RESPONSE_TIME        = 50U;  // ms
    static uint16_t const DEFAULT_DIAG_RESPONSE_PENDING     = 500U; // 10ms
    /* Programming session */
    static uint16_t const PROGRAMMING_DIAG_RESPONSE_TIME    = 50U;   // ms
    static uint16_t const PROGRAMMING_DIAG_RESPONSE_PENDING = 5000U; // 10ms

    /* Reset times */
    /* ECUReset: HardReset -> 10 01 */
    static uint16_t const RESET_TIME_HARD = 1000U; // ms
    /* ECUReset: SoftReset -> 10 03 */
    static uint16_t const RESET_TIME_SOFT = 1000U; // ms
    /* ECUReset: EnableRapidPowerShutdown -> 10 04 */
    static uint8_t const SHUTDOWN_TIME    = 10U; // ms
    /* DSC Reset */
    static uint16_t const RESET_TIME_DSC  = 1000U; // ms
};
} /* namespace uds */
