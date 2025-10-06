// Copyright 2024 Accenture.

#pragma once

#include <mcu/mcu.h>

#include <platform/estdint.h>

namespace safety
{
namespace mpu
{
static constexpr uint8_t SMPU_ENTRYS = 16U;

enum class ReadAccess : uint8_t
{
    nR = 0,
    R  = 1
};

enum class WriteAccess : uint8_t
{
    nW = 0,
    W  = 1
};

enum class AccessSupervisorMode : uint8_t
{
    SM_RWX      = 0,
    SM_RX       = 1,
    SM_RW       = 2,
    SM_UserMode = 3
};

enum class AccessUserMode : uint8_t
{
    UM_nXnWnR = 0,
    UM_X      = 1,
    UM_W      = 2,
    UM_R      = 4,
    UM_XW     = UM_X | UM_W,
    UM_XR     = UM_X | UM_R,
    UM_RW     = UM_R | UM_W,
    UM_RWX    = UM_R | UM_W | UM_X
};

enum class ProcessIdentifier : uint8_t
{
    nPI = 0,
    PI  = 1
};

union CESR
{
    uint32_t R;

    struct
    {
        uint32_t VLD            : 1;
        uint32_t Reserved_CESR1 : 7;
        uint32_t NRGD           : 4;
        uint32_t NSP            : 4;
        uint32_t HRL            : 4;
        uint32_t Reserved_CESR0 : 7;
        uint32_t SPERR4         : 1;
        uint32_t SPERR3         : 1;
        uint32_t SPERR2         : 1;
        uint32_t SPERR1         : 1;
        uint32_t SPERR0         : 1;
    } B;
};

struct Descriptor
{
    uint32_t startAddress;
    uint32_t endAddress;

    union
    {
        uint32_t R;

        struct
        {
            uint32_t M0UM            : 3;
            uint32_t M0SM            : 2;
            uint32_t M0PE            : 1;
            uint32_t M1UM            : 3;
            uint32_t M1SM            : 2;
            uint32_t M1PE            : 1;
            uint32_t M2UM            : 3;
            uint32_t M2SM            : 2;
            uint32_t ReservedWord2_0 : 1;
            uint32_t M3UM            : 3;
            uint32_t M3SM            : 2;
            uint32_t ReservedWord2_1 : 1;
            uint32_t M4WE            : 1;
            uint32_t M4RE            : 1;
            uint32_t M5WE            : 1;
            uint32_t M5RE            : 1;
            uint32_t M6WE            : 1;
            uint32_t M6RE            : 1;
            uint32_t M7WE            : 1;
            uint32_t M7RE            : 1;
        } B;
    } word2;

    union
    {
        uint32_t R;

        struct
        {
            uint32_t PID           : 8;
            uint32_t PIDMASK       : 8;
            uint32_t ReservedWord3 : 15;
            uint32_t VLD           : 1;
        } B;
    } word3;
};

inline void globalDisable() { IP_MPU->CESR = MPU_CESR_VLD(0); }

inline void globalEnable() { IP_MPU->CESR = MPU_CESR_VLD(1); }

inline bool isGlobalEnabled()
{
    return ((IP_MPU->CESR & (MPU_CESR_NRGD_MASK | MPU_CESR_VLD_MASK)) == 0x201U);
}

inline void setDescriptor(size_t const slot, Descriptor const& d)
{
    IP_MPU->RGD[slot].WORD0 = d.startAddress;
    IP_MPU->RGD[slot].WORD1 = d.endAddress;
    IP_MPU->RGD[slot].WORD2 = d.word2.R;
    IP_MPU->RGD[slot].WORD3 = d.word3.R;
}

template<uint8_t slot, uint8_t wordNumber, uint32_t word>
void setWordInDescriptor()
{
    static_assert(slot <= SMPU_ENTRYS, "");
    if (wordNumber == 2U)
    {
        IP_MPU->RGD[slot].WORD2 = word;
    }
    else if (wordNumber == 3U)
    {
        IP_MPU->RGD[slot].WORD3 = word;
    }
    else
    {
        // intentionally left empty
    }
}

template<uint8_t wordNumber>
uint32_t getWordInDescriptor(uint8_t const slot)
{
    if (slot >= SMPU_ENTRYS)
    {
        return 0U;
    }
    switch (wordNumber)
    {
        case 0:
        {
            return IP_MPU->RGD[slot].WORD0;
        }
        case 1:
        {
            return IP_MPU->RGD[slot].WORD1;
        }
        case 2:
        {
            return IP_MPU->RGD[slot].WORD2;
        }
        case 3:
        {
            return IP_MPU->RGD[slot].WORD3;
        }
        default:
        {
            return 0U;
        }
    }
}
}; // namespace mpu

constexpr uint32_t mpuWord2(
    mpu::ReadAccess m7re,
    mpu::WriteAccess m7we,
    mpu::ReadAccess m6re,
    mpu::WriteAccess m6we,
    mpu::ReadAccess m5re,
    mpu::WriteAccess m5we,
    mpu::ReadAccess m4re,
    mpu::WriteAccess m4we,
    mpu::AccessSupervisorMode m3sm,
    mpu::AccessUserMode m3um,
    mpu::AccessSupervisorMode m2sm,
    mpu::AccessUserMode m2um,
    mpu::ProcessIdentifier m1pe,
    mpu::AccessSupervisorMode m1sm,
    mpu::AccessUserMode m1um,
    mpu::ProcessIdentifier m0pe,
    mpu::AccessSupervisorMode m0sm,
    mpu::AccessUserMode m0um)
{
    return (static_cast<uint32_t>(m7re) << 31) | (static_cast<uint32_t>(m7we) << 30)
           | (static_cast<uint32_t>(m6re) << 29) | (static_cast<uint32_t>(m6we) << 28)
           | (static_cast<uint32_t>(m5re) << 27) | (static_cast<uint32_t>(m5we) << 26)
           | (static_cast<uint32_t>(m4re) << 25) | (static_cast<uint32_t>(m4we) << 24)
           | (static_cast<uint32_t>(m3sm) << 21) | (static_cast<uint32_t>(m3um) << 18)
           | (static_cast<uint32_t>(m2sm) << 15) | (static_cast<uint32_t>(m2um) << 12)
           | (static_cast<uint32_t>(m1pe) << 11) | (static_cast<uint32_t>(m1sm) << 9)
           | (static_cast<uint32_t>(m1um) << 6) | (static_cast<uint32_t>(m0pe) << 5)
           | (static_cast<uint32_t>(m0sm) << 3) | (static_cast<uint32_t>(m0um));
}

constexpr uint32_t mpuWord3(uint32_t pid, uint32_t pidmask, uint32_t vld)
{
    return (static_cast<uint32_t>(pid) << 24) | (static_cast<uint32_t>(pidmask) << 16)
           | (static_cast<uint32_t>(vld));
}

} // namespace safety
