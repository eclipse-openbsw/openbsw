// Copyright 2025 Accenture.

#include "safeMemory/RomCheck.h"

#include "safeMemory/SafetyShell.h"

#include <mcu/mcu.h>
#include <safeSupervisor/SafeSupervisor.h>
#include <safeUtils/SafetyLogger.h>

extern uintptr_t const __checksum_list_start[];
extern uint32_t const __checksum_result;

namespace safety
{
using ::util::logger::Logger;
using ::util::logger::SAFETY;

RomCheck::Result RomCheck::_result = RomCheck::Result::ROMCHECK_REQUESTED;

void RomCheck::init()
{
    SafetyShell const safetyShell;
    _result = RomCheck::Result::ROMCHECK_REQUESTED;
}

void RomCheck::idle()
{
    if (_result == Result::ROMCHECK_REQUESTED)
    {
        CRC_Type& crcRegisters        = *CRC;
        constexpr uint32_t seedValue  = 0xFFFFFFFFU;
        constexpr uint32_t polynomial = 0x04C11DB7U;

        // Initialize CRC module
        crcRegisters.CTRL = CRC_CTRL_TCRC(1);                    // 32-bit CRC
        crcRegisters.CTRL = crcRegisters.CTRL | CRC_CTRL_TOT(1); // Transpose bits for writing
        crcRegisters.CTRL
            = crcRegisters.CTRL | CRC_CTRL_TOTR(2); // Transpose bits & bytes for reading
        crcRegisters.CTRL  = crcRegisters.CTRL | CRC_CTRL_FXOR(1); // Final XOR
        crcRegisters.GPOLY = polynomial;

        // Write seed value
        crcRegisters.CTRL       = crcRegisters.CTRL | CRC_CTRL_WAS(1);
        crcRegisters.DATAu.DATA = seedValue;
        crcRegisters.CTRL
            = crcRegisters.CTRL
              & ~(CRC_CTRL_WAS(1)); // Clear CRC_CTRL[WAS] to start writing data values to the CRC.

        uint32_t const numOfAreas = __checksum_list_start[0];

        for (size_t i = 0U; i < numOfAreas; ++i)
        {
            auto const startAddress
                = reinterpret_cast<uintptr_t const*>(__checksum_list_start[i * 2 + 1]);
            auto const endAddress
                = reinterpret_cast<uintptr_t const*>(__checksum_list_start[i * 2 + 2]);

            for (uintptr_t const* p = startAddress; p < endAddress; ++p)
            {
                crcRegisters.DATAu.DATA = static_cast<uint32_t>(*p); // Write area data to the CRC
            }
        }

        // check result
        {
            SafetyShell const
                safetyShell; // Enter safety shell, used to grant temporary access to safety RAM
            bool const crc_valid
                = (__checksum_result
                   == crcRegisters.DATAu
                          .DATA); // After all values have been written to the CRC, the final result
                                  // can be read from CRC_DATA register.
            if (crc_valid)
            {
                Logger::debug(SAFETY, "ROM check passed successfully");
                _result = Result::ROMCHECK_OK;
            }
            else
            {
                auto& supervisor = safety::SafeSupervisor::getInstance();
                supervisor.romCheckMonitor.trigger();
                _result = Result::ROMCHECK_FAILED;
            }
        }
    }
}

} // namespace safety
