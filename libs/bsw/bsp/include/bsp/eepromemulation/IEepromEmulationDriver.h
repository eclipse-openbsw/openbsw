// Copyright 2024 Accenture.

#pragma once

#include <platform/estdint.h>

namespace eepromemulation
{
class IEepromEmulationDriver
{
public:
    enum EepromEmulationReturnCode
    {
        EE_OK                        = 0x00000000U,
        EE_FIRST_TIME_INITIALIZATION = 0x00000001U,
        EE_INFO_HVOP_INPROGRESS      = 0x00000002U,
        EE_INFO_PROGRAM_SUSPEND      = 0x00000004U,
        EE_INFO_ERASE_SUSPEND        = 0x00000010U,
        EE_ERROR_WRITE_IN_PROGRESS   = 0x00000020U,
        EE_ERROR_PE_OPT              = 0x00000040U,
        EE_ERROR_MISMATCH            = 0x00000080U,
        EE_ERROR_BLOCK_STATUS        = 0x00000100U,
        EE_ERROR_RECORD_STATUS       = 0x00000200U,
        EE_ERROR_BLOCK_CONFIG        = 0x00000400U,
        EE_ERROR_DATA_NOT_FOUND      = 0x00000800U,
        EE_ERROR_NOT_IN_CACHE        = 0x00001000U,
        EE_ERROR_NO_ENOUGH_SPACE     = 0x00002000U,
        EE_ERROR_CRC_MISMATCH        = 0x00004000U,
        EE_BUSY                      = 0x40000000U,
        EE_NOK                       = 0x80000000U,

        INVALID_CONFIG = 0x80000001U,

        READ_FAILED_INVALID_CONFIG = 0x80000002U,
        READ_FAILED_INVALID_ID     = 0x80000003U,

        WRITE_FAILED_INVALID_CONFIG                          = 0x80000004U,
        WRITE_FAILED_INVALID_ID                              = 0x80000005U,
        WRITE_FAILED_BUSY                                    = 0x80000006U,
        WRITE_FAILED_SIZE_TOO_LARGE                          = 0x80000007U,
        WRITE_FAILED_FLASH_ERASE_FAILED                      = 0x80000008U,
        WRITE_FAILED_FLASH_WRITE_FAILED                      = 0x80000009U,
        WRITE_FAILED_FLASH_FLUSH_FAILED                      = 0x8000000AU,
        WRITE_FAILED_INVALID_FLASH_BLOCK_ERASE_CYCLES_NUMBER = 0x8000000BU,

        FLASH_BLOCK1_ERASE_FAILED = 0x8000000CU,
        FLASH_BLOCK2_ERASE_FAILED = 0x8000000DU
    };

    virtual EepromEmulationReturnCode init(bool wakeUp) = 0;

    virtual EepromEmulationReturnCode read(uint16_t dataId, uint8_t* buffer, uint16_t& size) = 0;

    virtual EepromEmulationReturnCode write(uint16_t dataId, uint8_t const* buffer, uint16_t size)
        = 0;

    virtual EepromEmulationReturnCode write2(
        uint16_t dataId1,
        uint16_t dataId2,
        uint8_t const* buffer1,
        uint8_t const* buffer2,
        uint16_t size1,
        uint16_t size2)
        = 0;

protected:
    IEepromEmulationDriver& operator=(IEepromEmulationDriver const&) = default;
};

} /* namespace eepromemulation */
