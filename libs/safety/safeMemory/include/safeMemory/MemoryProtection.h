// Copyright 2024 Accenture.

#ifndef GUARD_DD12C93C_B902_4A16_90B1_408ED48FD742
#define GUARD_DD12C93C_B902_4A16_90B1_408ED48FD742

#include "safeMemory/mpu.h"

namespace safety
{
class MemoryProtection
{
public:
    static void init();

    static void fusaGateOpen();
    static void fusaGateClose();

    static bool fusaGateGetLockAndUnlock();
    static void fusaGateRestoreLock(bool gateLocked);

    static bool fusaGateIsLocked();

    static bool areRegionsConfiguredCorrectly(uint8_t& failed_region);

    static uint8_t const FUSA_REGION = 2U;

private:
    static bool checkLockProtection(uint8_t region);
    static bool checkAdresses(uint8_t region);
    static bool checkValidity(uint8_t region);

private:
    enum RegionDescriptor
    {
        RegionDescriptor_StartAddress  = 0U,
        RegionDescriptor_EndAddress    = 1U,
        RegionDescriptor_AccessControl = 2U,
        RegionDescriptor_Validity      = 3U
    };
};

} // namespace safety

#endif /* GUARD_DD12C93C_B902_4A16_90B1_408ED48FD742 */
