// Copyright 2025 Accenture.

#include "eeprom/EepromDriver.h"

#include <cache/cache.h>
#include <etl/memory.h>
#include <interrupts/suspendResumeAllInterrupts.h>

namespace eeprom
{

// EEPROM commands
static constexpr uint8_t PGMPART = 0x80U;
static constexpr uint8_t SETRAM  = 0x81U;

/* The timeout value should be large enough to be reached only in case of a serious issue,
 * i.e. after about one second or more. The current value should be safe, although the resulting
 * delay in seconds will vary depending on the clock settings, task priority, CPU load, etc. */
static constexpr uint32_t DRIVER_TIMEOUT = 0xFFFFFFU;

EepromDriver::EepromDriver(EepromConfiguration const& configuration) : _configuration(configuration)
{}

bool EepromDriver::isOperationAllowed(
    uint32_t const address, uint8_t const* const buffer, uint32_t const length) const
{
    if (buffer == nullptr)
    {
        return false;
    }

    if ((length == 0U) || (address >= _configuration.fTotalSize)
        || (length > (_configuration.fTotalSize - address)))
    {
        return false;
    }

    // Check that no flash command is already being processed. This should never happen, as the
    // Flash/FlexRAM drivers must not be used in parallel anyway. The purpose of this check isn't
    // to support parallel use, but to detect problems early.
    uint32_t timeout = 0U;
    while ((0U == (FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK))
           || (0U == (FTFC->FCNFG & FTFC_FCNFG_EEERDY_MASK)))
    {
        ++timeout;
        if (timeout > DRIVER_TIMEOUT)
        {
            return false;
        }
    }

    return true;
}

void EepromDriver::clearCommand() { _cmd.fill(0U); }

bool EepromDriver::isInitialized() const
{
    return (
        (((SIM->FCFG1 & SIM_FCFG1_EEERAMSIZE_MASK) >> SIM_FCFG1_EEERAMSIZE_SHIFT)
         == _configuration.fDataSetSize)
        && (((SIM->FCFG1 & SIM_FCFG1_DEPART_MASK) >> SIM_FCFG1_DEPART_SHIFT)
            == _configuration.fPartitionCode));
}

__attribute__((aligned(32))) uint8_t launchCommandFromRAM()
{
    uint32_t timeout             = 0U;
    constexpr uint8_t CMD_FAILED = 0xFFU;

    // Make sure previous command is completed
    while (0U == (FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK))
    {
        ++timeout;
        if (timeout > DRIVER_TIMEOUT)
        {
            return CMD_FAILED;
        }
    }

    // Clear Access and Protection Violation errors
    FTFC->FSTAT = FTFC_FSTAT_ACCERR(1) | FTFC_FSTAT_FPVIOL(1);

    // Launch command
    FTFC->FSTAT = FTFC_FSTAT_CCIF(1);

    timeout = 0U;
    while (0U == (FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK))
    {
        ++timeout;
        if (timeout > DRIVER_TIMEOUT)
        {
            return CMD_FAILED;
        }
    }

    return FTFC->FSTAT;
}

bsp::BspReturnCode EepromDriver::launchCommand()
{
    cacheInvalidate(0U, 0U);
    cacheDisable();
    auto const oldInterruptStatus = getMachineStateRegisterValueAndSuspendAllInterrupts();

    // Copy prepared command to FCCOB
    for (uint8_t i = 0U; i < FTFC_FCCOB_COUNT; i = (i + 4U))
    {
        FTFC->FCCOB[i]      = _cmd[i + 3U];
        FTFC->FCCOB[i + 1U] = _cmd[i + 2U];
        FTFC->FCCOB[i + 2U] = _cmd[i + 1U];
        FTFC->FCCOB[i + 3U] = _cmd[i];
    }

    /* The maximum possible size of the launchCommandFromRAM function is 0x88 bytes,
     * which was checked with different compiler optimization options.
     * Use slightly bigger RAM buffer to store the function */
    constexpr size_t FUNCTION_BUF_SIZE = 0x100U;
    __attribute__((aligned(32))) ::etl::array<uint8_t, FUNCTION_BUF_SIZE> copyOfLaunchFunction;

    /* Some EEPROM commands (ex. Program partition) must not be launched from flash memory,
     * since flash memory resources are not accessible during command execution.
     * That is why we launch command from RAM.
     * NOTE: Also make sure to clear the Thumb state bit to get the actual function address. */
    auto const* funPtr
        = reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(&launchCommandFromRAM) & ~1U);

    (void)::etl::mem_copy(funPtr, copyOfLaunchFunction.size(), copyOfLaunchFunction.data());

    uint8_t (*f)()    = reinterpret_cast<uint8_t (*)()>(&copyOfLaunchFunction[1U]);
    auto const result = f();

    cacheEnable();
    resumeAllInterrupts(oldInterruptStatus);

    if (0U
        == (result & (FTFC_FSTAT_MGSTAT0_MASK | FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK)))
    {
        return bsp::BSP_OK;
    }
    else
    {
        return bsp::BSP_ERROR;
    }
}

bsp::BspReturnCode EepromDriver::init()
{
    auto result = bsp::BSP_OK;

    if (!isInitialized())
    {
        clearCommand();
        _cmd[0U] = PGMPART;
        _cmd[1U] = static_cast<uint8_t>(
            _configuration.fProtectedAreaSize & 0x00000003UL); // CSEc Key Size
        _cmd[2U] = static_cast<uint8_t>(_configuration.fProtectedAreaSize & 0x00000004UL); // SFE
        _cmd[3U] = static_cast<uint8_t>(_configuration.fLoadOnReset);
        _cmd[4U] = _configuration.fDataSetSize;   // EEPROM Data Set Size Code
        _cmd[5U] = _configuration.fPartitionCode; // FlexNVM Partition Code
        result   = launchCommand();

        // Make sure the driver is initialized now
        if (!isInitialized())
        {
            return bsp::BSP_ERROR;
        }
    }

    if (bsp::BSP_OK == result)
    {
        clearCommand();
        _cmd[0U] = SETRAM;
        _cmd[1U] = 0U; // Make FlexRAM available for emulated EEPROM
        result   = launchCommand();
    }

    return result;
}

bsp::BspReturnCode
EepromDriver::write(uint32_t const address, uint8_t const* const buffer, uint32_t const length)
{
    if (!isOperationAllowed(address, buffer, length))
    {
        return bsp::BSP_ERROR;
    }

    auto* const target = reinterpret_cast<uint8_t*>(address + _configuration.fBaseAddress);
    for (uint32_t i = 0U; i < length; ++i)
    {
        target[i] = buffer[i];

        uint32_t timeout = 0U;
        while (0U == (FTFC->FCNFG & FTFC_FCNFG_EEERDY_MASK))
        {
            ++timeout;
            if (timeout > DRIVER_TIMEOUT)
            {
                return bsp::BSP_ERROR;
            }
        }
    }

    return bsp::BSP_OK;
}

bsp::BspReturnCode
EepromDriver::read(uint32_t const address, uint8_t* const buffer, uint32_t const length)
{
    if (!isOperationAllowed(address, buffer, length))
    {
        return bsp::BSP_ERROR;
    }

    auto* const source = reinterpret_cast<uint8_t*>(address + _configuration.fBaseAddress);
    for (uint32_t i = 0U; i < length; ++i)
    {
        buffer[i] = source[i];
    }

    // Check for read collision error (not supposed to happen, but just in case)
    if (0U != (FTFC->FSTAT & FTFC_FSTAT_RDCOLERR_MASK))
    {
        FTFC->FSTAT = FTFC_FSTAT_RDCOLERR(1); // Clear error
        return bsp::BSP_ERROR;
    }

    return bsp::BSP_OK;
}

} // namespace eeprom
