// Copyright 2024 Accenture.

#ifndef GUARD_DEA0839B_70CB_4E6F_956A_4C9B8792E737
#define GUARD_DEA0839B_70CB_4E6F_956A_4C9B8792E737

#include <platform/estdint.h>

namespace uds
{
class UdsVmsConstants
{
public:
    static uint16_t const MAX_BLOCK_LENGTH = 4090U;

    static uint16_t const ERASE_MEMORY_TIME            = 120U;
    static uint16_t const CHECK_MEMORY_TIME            = 15U;
    static uint16_t const BOOTLOADER_INSTALLATION_TIME = 40U;
    static uint16_t const AUTHENTICATION_TIME          = 5U;

    static uint16_t const RESET_TIME = 10U;

    static uint16_t const TRANSFER_DATA_TIME        = 60U;
    static uint32_t const TESTER_PRESENT_TIMEOUT_MS = 5000U;

    static uint8_t const BUSY_MESSAGE_EXTRA_BYTES = 7U;
};

} /* namespace uds */

#endif /* GUARD_DEA0839B_70CB_4E6F_956A_4C9B8792E737 */
