// Copyright 2024 Accenture.

#include "safeMemory/MemoryProtection.h"

#include "safeMemory/MemoryLocations.h"
#include "safeMemory/mpu.h"

#include <interrupts/SuspendResumeAllInterruptsScopedLock.h>

namespace safety
{
static constexpr uint32_t REGION_FULL_ACCESS = mpuWord2(
    mpu::ReadAccess::R,
    mpu::WriteAccess::W,
    mpu::ReadAccess::R,
    mpu::WriteAccess::W,
    mpu::ReadAccess::R,
    mpu::WriteAccess::W,
    mpu::ReadAccess::R,
    mpu::WriteAccess::W,
    mpu::AccessSupervisorMode::SM_RWX,
    mpu::AccessUserMode::UM_RWX,
    mpu::AccessSupervisorMode::SM_RWX,
    mpu::AccessUserMode::UM_RWX,
    mpu::ProcessIdentifier::nPI,
    mpu::AccessSupervisorMode::SM_UserMode,
    mpu::AccessUserMode::UM_RWX,
    mpu::ProcessIdentifier::nPI,
    mpu::AccessSupervisorMode::SM_UserMode,
    mpu::AccessUserMode::UM_RWX);
static constexpr uint32_t REGION_VALID              = mpuWord3(0, 0, 1);
static constexpr uint32_t REGION_LOCKED_READ_ACCESS = mpuWord2(
    mpu::ReadAccess::nR,
    mpu::WriteAccess::nW,
    mpu::ReadAccess::nR,
    mpu::WriteAccess::nW,
    mpu::ReadAccess::nR,
    mpu::WriteAccess::nW,
    mpu::ReadAccess::nR,
    mpu::WriteAccess::nW,
    mpu::AccessSupervisorMode::SM_UserMode,
    mpu::AccessUserMode::UM_nXnWnR,
    mpu::AccessSupervisorMode::SM_UserMode,
    mpu::AccessUserMode::UM_nXnWnR,
    mpu::ProcessIdentifier::nPI,
    mpu::AccessSupervisorMode::SM_UserMode,
    mpu::AccessUserMode::UM_R,
    mpu::ProcessIdentifier::nPI,
    mpu::AccessSupervisorMode::SM_UserMode,
    mpu::AccessUserMode::UM_R);

static constexpr uint8_t numOfMemoryProtectionConfigurationRamRegions = 5;

// linker script asserts that .data and .rodata are empty
static mpu::Descriptor __attribute__((section(".bss")))
memoryProtectionConfigurationRam[numOfMemoryProtectionConfigurationRamRegions];

void MemoryProtection::init()
{
    mpu::globalDisable();
    /* the cortex-m microcontroller has by default the Debugger Bus Master active in region 0 and
       this cannot be changed. Therefore this Region is configured with no Read/Write/Execute access
       for the Core Bus and implicitly deactivated.*/
    // clang-format off
    mpu::setWordInDescriptor<0U, RegionDescriptor_AccessControl,
            mpuWord2(mpu::ReadAccess::R, mpu::WriteAccess::W, mpu::ReadAccess::R, mpu::WriteAccess::W, mpu::ReadAccess::R, mpu::WriteAccess::W, mpu::ReadAccess::R, mpu::WriteAccess::W,
            mpu::AccessSupervisorMode::SM_RWX, mpu::AccessUserMode::UM_RWX, mpu::AccessSupervisorMode::SM_RWX, mpu::AccessUserMode::UM_RWX,
            mpu::ProcessIdentifier::nPI, mpu::AccessSupervisorMode::SM_UserMode, mpu::AccessUserMode::UM_RWX,
            mpu::ProcessIdentifier::nPI, mpu::AccessSupervisorMode::SM_UserMode, mpu::AccessUserMode::UM_nXnWnR)>();

    uint32_t const PROTECTED_RAM_START_ADDR_MINUS_1
        = reinterpret_cast<uintptr_t>(__MPU_BSS_START) - 1U;
    uint32_t const PROTECTED_RAM_END_ADDR = reinterpret_cast<uintptr_t>(__MPU_BSS_END) - 1U;
    uint32_t const SAFETY_TASK_START_ADDR = reinterpret_cast<uint32_t>(&safetyStack[0]);
    uint32_t const SAFETY_TASK_END_ADDR
        = SAFETY_TASK_START_ADDR + (static_cast<uint32_t>(safety_task_stackSize) - 1U);

    memoryProtectionConfigurationRam[0] =
        {0x0000000U,                            PROTECTED_RAM_START_ADDR_MINUS_1, {REGION_FULL_ACCESS},        {REGION_VALID}};
    memoryProtectionConfigurationRam[1] =
        {PROTECTED_RAM_START_ADDR_MINUS_1 + 1U, PROTECTED_RAM_END_ADDR,           {REGION_LOCKED_READ_ACCESS}, {REGION_VALID}};
    memoryProtectionConfigurationRam[2] =
        {PROTECTED_RAM_END_ADDR + 1U,           SAFETY_TASK_START_ADDR - 1U,      {REGION_FULL_ACCESS},        {REGION_VALID}};
    memoryProtectionConfigurationRam[3] =
        {SAFETY_TASK_START_ADDR,                SAFETY_TASK_END_ADDR,             {REGION_FULL_ACCESS},        {REGION_VALID}};
    memoryProtectionConfigurationRam[4] =
        {SAFETY_TASK_END_ADDR + 1U,             0xFFFFFFFFU,                      {REGION_FULL_ACCESS},        {REGION_VALID}};
    // clang-format on

    for (size_t i = 0U; i < numOfMemoryProtectionConfigurationRamRegions; ++i)
    {
        mpu::setDescriptor(i + 1U, (mpu::Descriptor const&)memoryProtectionConfigurationRam[i]);
    }
    mpu::globalEnable();
}

void MemoryProtection::fusaGateOpen()
{
    mpu::setWordInDescriptor<FUSA_REGION, RegionDescriptor_AccessControl, REGION_FULL_ACCESS>();
    mpu::setWordInDescriptor<FUSA_REGION, RegionDescriptor_Validity, REGION_VALID>();
}

void MemoryProtection::fusaGateClose()
{
    mpu::setWordInDescriptor<
        FUSA_REGION,
        RegionDescriptor_AccessControl,
        REGION_LOCKED_READ_ACCESS>();
    mpu::setWordInDescriptor<FUSA_REGION, RegionDescriptor_Validity, REGION_VALID>();
}

bool MemoryProtection::fusaGateGetLockAndUnlock()
{
    // the lock is needed here (although it's also done before calling this function)
    // otherwise compiler will break the code when optimizing assembler and C++ code
    ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;
    bool const gateLocked = fusaGateIsLocked();
    mpu::setWordInDescriptor<FUSA_REGION, RegionDescriptor_AccessControl, REGION_FULL_ACCESS>();
    mpu::setWordInDescriptor<FUSA_REGION, RegionDescriptor_Validity, REGION_VALID>();
    return gateLocked;
}

void MemoryProtection::fusaGateRestoreLock(bool const gateLocked)
{
    if (gateLocked)
    {
        mpu::setWordInDescriptor<
            FUSA_REGION,
            RegionDescriptor_AccessControl,
            REGION_LOCKED_READ_ACCESS>();
    }
    else
    {
        mpu::setWordInDescriptor<FUSA_REGION, RegionDescriptor_AccessControl, REGION_FULL_ACCESS>();
    }
    mpu::setWordInDescriptor<FUSA_REGION, RegionDescriptor_Validity, REGION_VALID>();
}

bool MemoryProtection::fusaGateIsLocked()
{
    uint32_t const lockWord = mpu::getWordInDescriptor<RegionDescriptor_AccessControl>(FUSA_REGION);
    return (lockWord == REGION_LOCKED_READ_ACCESS);
}

void MemoryProtection::fusaTaskStackCloseIsr()
{
    mpu::setWordInDescriptor<
        SAFETY_STACK_REGION,
        RegionDescriptor_AccessControl,
        REGION_LOCKED_READ_ACCESS>();
    mpu::setWordInDescriptor<SAFETY_STACK_REGION, RegionDescriptor_Validity, REGION_VALID>();
}

void MemoryProtection::fusaTaskStackOpenIsr()
{
    mpu::setWordInDescriptor<
        SAFETY_STACK_REGION,
        RegionDescriptor_AccessControl,
        REGION_FULL_ACCESS>();
    mpu::setWordInDescriptor<SAFETY_STACK_REGION, RegionDescriptor_Validity, REGION_VALID>();
}

bool MemoryProtection::checkLockProtection(uint8_t const region)
{
    constexpr uint32_t mask_available_regions = 0x0001FFFFU;

    uint32_t const current_lock_word
        = mpu::getWordInDescriptor<RegionDescriptor_AccessControl>(region + 1);
    if ((region + 1) == MemoryProtection::FUSA_REGION)
    {
        // always checked in open stated
        if ((current_lock_word & mask_available_regions)
            != (REGION_FULL_ACCESS & mask_available_regions))
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

bool MemoryProtection::checkAddresses(uint8_t const region)
{
    if (numOfMemoryProtectionConfigurationRamRegions <= region)
    {
        return false;
    }

    uint32_t const start_address
        = mpu::getWordInDescriptor<RegionDescriptor_StartAddress>(region + 1);
    uint32_t const end_address = mpu::getWordInDescriptor<RegionDescriptor_EndAddress>(region + 1);

    static constexpr uint32_t MASK_START_ADDRESS = 0xFFFFFFE0U;
    static constexpr uint32_t MASK_END_ADDRESS   = 0x0000001FU;

    uint32_t const config_start_address
        = (memoryProtectionConfigurationRam[region].startAddress & MASK_START_ADDRESS);
    uint32_t const config_end_address
        = (memoryProtectionConfigurationRam[region].endAddress | MASK_END_ADDRESS);

    if ((start_address != config_start_address) || (end_address != config_end_address))
    {
        return false;
    }
    return true;
}

bool MemoryProtection::checkValidity(uint8_t const region)
{
    uint32_t const validity = mpu::getWordInDescriptor<RegionDescriptor_Validity>(region + 1);
    if ((memoryProtectionConfigurationRam[region].word3.R & REGION_VALID)
        != (validity & REGION_VALID))
    {
        return false;
    }
    return true;
}

bool MemoryProtection::areRegionsConfiguredCorrectly(uint8_t& failed_region)
{
    constexpr uint8_t MAX_MPU_REGIONS = 16U;

    for (uint8_t i = 0U; i < numOfMemoryProtectionConfigurationRamRegions; ++i)
    {
        if (!MemoryProtection::checkAddresses(i))
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
    for (uint8_t i = numOfMemoryProtectionConfigurationRamRegions + 1; i < MAX_MPU_REGIONS; ++i)
    {
        uint32_t const validity
            = (mpu::getWordInDescriptor<RegionDescriptor_Validity>(i) & REGION_VALID);
        if (validity != 0U)
        {
            return false;
        }
    }
    return true;
}
} // namespace safety
