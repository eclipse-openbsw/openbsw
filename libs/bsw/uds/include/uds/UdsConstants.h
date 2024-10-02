// Copyright 2024 Accenture.

#ifndef GUARD_DA66F97C_C8A9_4826_9CBD_F1E24E375E4B
#define GUARD_DA66F97C_C8A9_4826_9CBD_F1E24E375E4B

#include "platform/estdint.h"

namespace uds
{
class ServiceId
{
    ServiceId();

public:
    static constexpr uint8_t OBD_MODE_1                         = 0x01U;
    static constexpr uint8_t OBD_MODE_2                         = 0x02U;
    static constexpr uint8_t OBD_MODE_3                         = 0x03U;
    static constexpr uint8_t OBD_MODE_4                         = 0x04U;
    static constexpr uint8_t OBD_MODE_7                         = 0x07U;
    static constexpr uint8_t OBD_MODE_9                         = 0x09U;
    static constexpr uint8_t OBD_MODE_A                         = 0x0AU;
    static constexpr uint8_t DIAGNOSTIC_SESSION_CONTROL         = 0x10U;
    static constexpr uint8_t ECU_RESET                          = 0x11U;
    static constexpr uint8_t CLEAR_DIAGNOSTIC_INFORMATION       = 0x14U;
    static constexpr uint8_t READ_DTC_INFORMATION               = 0x19U;
    static constexpr uint8_t READ_DATA_BY_IDENTIFIER            = 0x22U;
    static constexpr uint8_t READ_MEMORY_BY_ADDRESS             = 0x23U;
    static constexpr uint8_t READ_SCALING_DATA_BY_IDENTIFIER    = 0x24U;
    static constexpr uint8_t SECURITY_ACCESS                    = 0x27U;
    static constexpr uint8_t COMMUNICATION_CONTROL              = 0x28U;
    static constexpr uint8_t AUTHENTICATION                     = 0x29U;
    static constexpr uint8_t READ_DATA_BY_PERIODIC_IDENTIFIER   = 0x2AU;
    static constexpr uint8_t DYNAMICALLY_DEFINE_DATA_IDENTIFIER = 0x2CU;
    static constexpr uint8_t WRITE_DATA_BY_IDENTIFIER           = 0x2EU;
    static constexpr uint8_t INPUT_OUTPUT_CONTROL_BY_IDENTIFIER = 0x2FU;
    static constexpr uint8_t ROUTINE_CONTROL                    = 0x31U;
    static constexpr uint8_t REQUEST_DOWNLOAD                   = 0x34U;
    static constexpr uint8_t REQUEST_UPLOAD                     = 0x35U;
    static constexpr uint8_t TRANSFER_DATA                      = 0x36U;
    static constexpr uint8_t REQUEST_TRANSFER_EXIT              = 0x37U;
    static constexpr uint8_t REQUEST_FILE_TRANSFER              = 0x38U;
    static constexpr uint8_t WRITE_MEMORY_BY_ADDRESS            = 0x3DU;
    static constexpr uint8_t TESTER_PRESENT                     = 0x3EU;
    static constexpr uint8_t ACCESS_TIMING_PARAMETER            = 0x83U;
    static constexpr uint8_t SECURED_DATA_TRANSMISSION          = 0x84U;
    static constexpr uint8_t CONTROL_DTC_SETTING                = 0x85U;
    static constexpr uint8_t RESPONSE_ON_EVENT                  = 0x86U;
    static constexpr uint8_t LINK_CONTROl                       = 0x87U;
};

} // namespace uds

#endif // GUARD_DA66F97C_C8A9_4826_9CBD_F1E24E375E4B
