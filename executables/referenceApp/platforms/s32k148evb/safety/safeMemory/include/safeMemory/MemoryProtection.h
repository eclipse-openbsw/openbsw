// Copyright 2024 Accenture.

#pragma once

#include <platform/estdint.h>

namespace safety
{
class MemoryProtection
{
public:
    // [PUBLIC_API_START]
    /**
     * Defines the memory regions with distinct attributes and access permissions by setting
     * the region descriptors.
     */
    static void init();
    /**
     * Changes the access permission for protected RAM region(FUSA_REGION) to full
     * access(read/write/execute).
     */
    static void fusaGateOpen();
    /**
     * Changes the access permission for protected RAM region(FUSA_REGION) to read only
     * access(read/no write/no execute).
     */
    static void fusaGateClose();
    /**
     * Returns the access permission status of protected RAM region(FUSA_REGION) and provides
     * full access(read/write/execute).
     * \return true if FUSA_REGION has read only access
     * \return false if FUSA_REGION has full access
     */
    static bool fusaGateGetLockAndUnlock();
    /**
     * Restores the previous access right of the FUSA_REGION.
     * \param gateLocked stores the old access permission of FUSA_REGION
     */
    static void fusaGateRestoreLock(bool gateLocked);
    /**
     * Checks whether the FUSA_REGION has read only access(read/no write/no execute).
     * \return true if region is locked
     * \return false if region has full access
     */
    static bool fusaGateIsLocked();
    static void fusaTaskStackCloseIsr();
    static void fusaTaskStackOpenIsr();
    /**
     * Verifies whether all the memory regions are configured correctly.
     * \return true if the region is configured correctly
     * \return false if there is a deviation in configuration of the region
     */
    static bool areRegionsConfiguredCorrectly(uint8_t& failed_region);
    // [PUBLIC_API_END]
    static constexpr uint8_t FUSA_REGION         = 2U;
    static constexpr uint8_t SAFETY_STACK_REGION = 4U;

private:
    static bool checkLockProtection(uint8_t region);
    static bool checkAddresses(uint8_t region);
    static bool checkValidity(uint8_t region);

    enum RegionDescriptor
    {
        RegionDescriptor_StartAddress  = 0U,
        RegionDescriptor_EndAddress    = 1U,
        RegionDescriptor_AccessControl = 2U,
        RegionDescriptor_Validity      = 3U
    };
};

} // namespace safety
