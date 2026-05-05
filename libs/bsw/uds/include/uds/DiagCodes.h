// Copyright 2024 Accenture.
// Copyright 2026 BMW AG

#pragma once

#include "platform/estdint.h"

namespace uds
{
class DiagCodes
{
public:
    static uint8_t const POSITIVE_RESPONSE                        = 0x00U;
    static uint8_t const POSITIVE_RESPONSE_WAIT_FOR_EEPROM_FINISH = 0x00U;
    static uint8_t const MIN_DIAG_MESSAGE_LENGTH                  = 3U;
    static uint8_t const NEGATIVE_RESPONSE_MESSAGE_LENGTH         = 3U;
    static uint8_t const NEGATIVE_RESPONSE_IDENTIFIER_OFFSET      = 0U;
    static uint8_t const NEGATIVE_RESPONSE_SERVICE_OFFSET         = 1U;
    static uint8_t const NEGATIVE_RESPONSE_ERRORCODE_OFFSET       = 2U;

    // Diag identifier
    static uint8_t const ID_ROUTINE_CONTROL_START_ROUTINE      = 0x01U;
    static uint8_t const ID_ROUTINE_CONTROL_STOP_ROUTINE       = 0x02U;
    static uint8_t const ID_ROUTINE_CONTROL_GET_ROUTINE_RESULT = 0x03U;

    // Functional addressing
    static uint8_t const FUNCTIONAL_ID_ALL_KWP2000_ECUS  = 0xEFU;
    static uint8_t const FUNCTIONAL_ID_ALL_ISO14229_ECUS = 0xDFU;

    static uint8_t const SERVICE_ID_LENGTH     = 1U;
    static uint8_t const SUBFUNCTION_ID_LENGTH = 1U;
    static uint8_t const ROUTINE_ID_LENGTH     = 2U;
    static uint8_t const DATA_ID_LENGTH        = 2U;
};

} // namespace uds
