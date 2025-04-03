// Copyright 2024 Accenture.

#include "safeMemory/MemoryProtection.h"

#include "safeMemory/MemoryLocations.h"

#include <safeLifecycle/interrupts/IsrLock.h>

using safety::Mpu;

namespace safety
{
// clang-format off
static const uint32_t regionFullAccess = safety::mpuWord2<
    Mpu::R, Mpu::W, Mpu::R, Mpu::W, Mpu::R, Mpu::W, Mpu::R, Mpu::W,
    Mpu::SM_RWX, Mpu::UM_RWX, Mpu::SM_RWX, Mpu::UM_RWX,
    Mpu::nPI, Mpu::SM_UserMode, Mpu::UM_RWX,
    Mpu::nPI, Mpu::SM_UserMode, Mpu::UM_RWX>::word;
static const uint32_t regionValid = safety::mpuWord3<0, 0, 1>::word;
static const uint32_t regionLockedReadAccess = safety::mpuWord2<
    Mpu::nR, Mpu::nW, Mpu::nR, Mpu::nW, Mpu::nR, Mpu::nW, Mpu::nR, Mpu::nW,
    Mpu::SM_UserMode, Mpu::UM_nXnWnR, Mpu::SM_UserMode, Mpu::UM_nXnWnR,
    Mpu::nPI, Mpu::SM_UserMode, Mpu::UM_R,
    Mpu::nPI, Mpu::SM_UserMode, Mpu::UM_R>::word;

#ifdef PLATFORM_SUPPORT_MPU
static const uint32_t protectedRamStartAddrMinus1 = reinterpret_cast<uintptr_t>(__MPU_BSS_START) - 1U;
static const uint32_t protectedRamEndAddr         = reinterpret_cast<uintptr_t>(__MPU_BSS_END) - 1U;

static const Mpu::tDescriptor memoryProtectionConfigurationRam[] = {
    /*1*/ {0x0000000U,                       protectedRamStartAddrMinus1, {regionFullAccess},       {regionValid}},
    /*2*/ {protectedRamStartAddrMinus1 + 1U, protectedRamEndAddr,         {regionLockedReadAccess}, {regionValid}},
    /*3*/ {protectedRamEndAddr + 1U,         0xFFFFFFFFU,                 {regionFullAccess},       {regionValid}},
};

static const uint8_t numOfMemoryProtectionConfigurationRamRegions =
    static_cast<uint8_t>(sizeof(memoryProtectionConfigurationRam) / sizeof(memoryProtectionConfigurationRam[0]));
#endif

// clang-format on

void MemoryProtection::init()
{
    Mpu::globalDisable();
    // clang-format off
    Mpu::setWordInDescriptor<0U, RegionDescriptor_AccessControl,
        safety::mpuWord2<Mpu::R, Mpu::W, Mpu::R, Mpu::W, Mpu::R, Mpu::W, Mpu::R, Mpu::W,
            Mpu::SM_RWX, Mpu::UM_RWX, Mpu::SM_RWX, Mpu::UM_RWX,
            Mpu::nPI, Mpu::SM_UserMode, Mpu::UM_RWX,
            Mpu::nPI, Mpu::SM_UserMode, Mpu::UM_nXnWnR>::word>();
    // clang-format on
#ifdef PLATFORM_SUPPORT_MPU
    for (size_t i = 0U; i < numOfMemoryProtectionConfigurationRamRegions; ++i)
    {
        Mpu::setDescriptor(i + 1U, (Mpu::tDescriptor const&)memoryProtectionConfigurationRam[i]);
    }
#endif
    Mpu::globalEnable();
}

void MemoryProtection::fusaGateOpen()
{
    Mpu::setWordInDescriptor<FUSA_REGION, RegionDescriptor_AccessControl, regionFullAccess>();
    Mpu::setWordInDescriptor<FUSA_REGION, RegionDescriptor_Validity, regionValid>();
}

void MemoryProtection::fusaGateClose()
{
    Mpu::setWordInDescriptor<FUSA_REGION, RegionDescriptor_AccessControl, regionLockedReadAccess>();
    Mpu::setWordInDescriptor<FUSA_REGION, RegionDescriptor_Validity, regionValid>();
}

bool MemoryProtection::fusaGateGetLockAndUnlock()
{
    // the lock is needed here (although it's also done before calling this function)
    // otherwise compiler will break the code when optimizing assembler and C++ code
    safety::IsrLock const lock;
    bool const gateLocked = fusaGateIsLocked();
    Mpu::setWordInDescriptor<FUSA_REGION, RegionDescriptor_AccessControl, regionFullAccess>();
    Mpu::setWordInDescriptor<FUSA_REGION, RegionDescriptor_Validity, regionValid>();
    return gateLocked;
}

void MemoryProtection::fusaGateRestoreLock(bool const gateLocked)
{
    if (gateLocked)
    {
        Mpu::setWordInDescriptor<
            FUSA_REGION,
            RegionDescriptor_AccessControl,
            regionLockedReadAccess>();
    }
    else
    {
        Mpu::setWordInDescriptor<FUSA_REGION, RegionDescriptor_AccessControl, regionFullAccess>();
    }
    Mpu::setWordInDescriptor<FUSA_REGION, RegionDescriptor_Validity, regionValid>();
}

bool MemoryProtection::fusaGateIsLocked()
{
    uint32_t const lockWord = Mpu::getWordInDescriptor<RegionDescriptor_AccessControl>(FUSA_REGION);
    return (lockWord == regionLockedReadAccess);
}

#ifdef PLATFORM_SUPPORT_MPU
bool MemoryProtection::checkLockProtection(uint8_t const region)
{
    uint32_t const mask_available_regions = 0x0001FFFFU;

    uint32_t const current_lock_word
        = Mpu::getWordInDescriptor<RegionDescriptor_AccessControl>(region + 1);
    if ((region + 1) == MemoryProtection::FUSA_REGION)
    {
        // always checked in open stated
        if ((current_lock_word & mask_available_regions)
            != (regionFullAccess & mask_available_regions))
        {
            return false;
        }
    }
    else
    {
        if ((current_lock_word & mask_available_regions)
            != (memoryProtectionConfigurationRam[region].word2.R & mask_available_regions))
        {
            return false;
        }
    }
    return true;
}

bool MemoryProtection::checkAdresses(uint8_t const region)
{
    if (numOfMemoryProtectionConfigurationRamRegions <= region)
    {
        return false;
    }

    uint32_t const start_address
        = Mpu::getWordInDescriptor<RegionDescriptor_StartAddress>(region + 1);
    uint32_t const end_address = Mpu::getWordInDescriptor<RegionDescriptor_EndAddress>(region + 1);

    static uint32_t const MASK_START_ADDRESS = 0xFFFFFFE0U;
    static uint32_t const MASK_END_ADDRESS   = 0x0000001FU;

    uint32_t const config_start_address
        = (memoryProtectionConfigurationRam[region].startAdress & MASK_START_ADDRESS);
    uint32_t const config_end_address
        = (memoryProtectionConfigurationRam[region].endAdress | MASK_END_ADDRESS);

    if ((start_address != config_start_address) || (end_address != config_end_address))
    {
        return false;
    }
    return true;
}

bool MemoryProtection::checkValidity(uint8_t const region)
{
    uint32_t const validity = Mpu::getWordInDescriptor<RegionDescriptor_Validity>(region + 1);
    if ((memoryProtectionConfigurationRam[region].word3.R & regionValid)
        != (validity & regionValid))
    {
        return false;
    }
    return true;
}

bool MemoryProtection::areRegionsConfiguredCorrectly(uint8_t& failed_region)
{
    uint8_t const MAX_MPU_REGIONS = 8U;

    for (uint8_t i = 0U; i < numOfMemoryProtectionConfigurationRamRegions; i++)
    {
        if (!MemoryProtection::checkAdresses(i))
        {
            return false;
        }
        if (!MemoryProtection::checkLockProtection(i))
        {
            failed_region = i + 1;
            return false;
        }
        if (!MemoryProtection::checkValidity(i))
        {
            failed_region = i + 1;
            return false;
        }
    }
    for (uint8_t i = numOfMemoryProtectionConfigurationRamRegions + 1; i < MAX_MPU_REGIONS; i++)
    {
        uint32_t const validity
            = (Mpu::getWordInDescriptor<RegionDescriptor_Validity>(i) & regionValid);
        if (validity != 0U)
        {
            return false;
        }
    }
    return true;
}
#endif
} // namespace safety
