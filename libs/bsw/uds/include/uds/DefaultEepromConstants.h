// Copyright 2024 Accenture.

#pragma once

#include "platform/estdint.h"

namespace uds
{
/**
 * \see EepromTable
 */
class EepromConstants
{
public:
    static uint8_t const OFFSET_SESSION                  = 0U;
    static uint8_t const OFFSET_PROGRAMMING_DEPENDENCIES = 1U;
    static uint8_t const OFFSET_LIFECYCLE_MODE           = 2U;
    static uint8_t const OFFSET_FETRAFLA                 = 3U;

    /** Offset within SVK_STATUS_SECTION */
    static uint8_t const OFFSET_STATUS_SVK              = 0U;
    /** Offset within SVK_STATUS_SECTION */
    static uint8_t const OFFSET_PROGRAMMING_COUNTER     = 3U;
    /** Offset within SVK_STATUS_SECTION */
    static uint8_t const OFFSET_PROGRAMMING_COUNTER_MAX = 5U;
    /** Offset within SVK_STATUS_SECTION */
    static uint8_t const OFFSET_FINGERPRINT             = 7U;
};

} // namespace uds

