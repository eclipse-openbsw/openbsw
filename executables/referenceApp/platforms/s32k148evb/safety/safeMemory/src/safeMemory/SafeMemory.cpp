// Copyright 2025 Accenture.

#include "safeMemory/SafeMemory.h"

#include "safeMemory/IntegrityEccHandler.h"

#include <safeMemory/MemoryProtection.h>
#include <safeMemory/mpu.h>
#include <safeSupervisor/SafeSupervisor.h>
#include <safeUtils/SafetyLogger.h>

namespace safety
{
using ::util::logger::Logger;
using ::util::logger::SAFETY;

namespace safe_memory
{
void cyclic()
{
    if (!mpu::isGlobalEnabled())
    {
        auto& supervisor = safety::SafeSupervisor::getInstance();
        supervisor.mpuEnableMonitor.trigger(static_cast<uint32_t>(IP_MPU->CESR));
    }
    uint8_t failed_region = 0U;
    if (!MemoryProtection::areRegionsConfiguredCorrectly(failed_region))
    {
        auto& supervisor = safety::SafeSupervisor::getInstance();
        supervisor.mpuConfigMonitor.trigger(failed_region);
    }
    // check ecc errors
    auto const hasRamDoubleBitError   = checkRamDoubleBitError();
    auto const hasFlashDoubleBitError = checkFlashDoubleBitError();
    if (hasRamDoubleBitError || hasFlashDoubleBitError)
    {
        if (hasRamDoubleBitError)
        {
            Logger::debug(SAFETY, "ECC_RAM_DOUBLE_BIT_ERROR");
        }
        if (hasFlashDoubleBitError)
        {
            Logger::debug(SAFETY, "ECC_FLASH_DOUBLE_BIT_ERROR");
        }
        auto& supervisor = safety::SafeSupervisor::getInstance();
        supervisor.dmaEccMonitor.trigger();
    }
}

void checkEccErrors()
{
    auto const hasRamDoubleBitError = checkRamDoubleBitError();
    if (hasRamDoubleBitError)
    {
        auto errorAddress = readErmMemoryErrorAddress();
        Logger::debug(SAFETY, "ECC_RAM_DOUBLE_BIT_ERROR at address 0x%08X", errorAddress);
    }
    // TODO: remove the log, and replace it with writing the event to no-init ram
}

} // namespace safe_memory
} // namespace safety
