// Copyright 2024 Accenture.

#pragma once

#ifdef PLATFORM_SUPPORT_MPU
#include <mcu/mcu.h>
#endif

#include <estd/assert.h>
#include <platform/estdint.h>

namespace safety
{
class Mpu
{
    static uint8_t const SMPU_ENTRYS = 16U;
    static uint8_t const SMPUs       = 2U;

public:
    typedef enum
    {
        FLASH_PER_RANGE = 0,
        RAM_RANGE
    } tItem;

    typedef enum
    {
        nR = 0,
        R  = 1
    } tReadAccess;

    typedef enum
    {
        nW = 0,
        W  = 1
    } tWriteAccess;

    typedef enum
    {
        SM_RWX      = 0,
        SM_RX       = 1,
        SM_RW       = 2,
        SM_UserMode = 3
    } tAccessSupervisorMode;

    typedef enum
    {
        UM_nXnWnR = 0,
        UM_X      = 1,
        UM_W      = 2,
        UM_R      = 4,
        UM_XW     = UM_X | UM_W,
        UM_XR     = UM_X | UM_R,
        UM_RW     = UM_R | UM_W,
        UM_RWX    = UM_R | UM_W | UM_X
    } tAccessUserMode;

    typedef enum
    {
        nPI = 0,
        PI  = 1
    } tProcessIdentifier;

    typedef uint8_t tPid;
    typedef uint8_t tPidMask;

    typedef union
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
    } CESR;

    typedef struct Descriptor
    {
        uint32_t startAdress;
        uint32_t endAdress;

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
    } tDescriptor;

    static void globalDisable()
    {
#ifdef PLATFORM_SUPPORT_MPU
        IP_MPU->CESR = MPU_CESR_VLD(0);
#endif
    }

    static void globalEnable()
    {
#ifdef PLATFORM_SUPPORT_MPU
        IP_MPU->CESR = MPU_CESR_VLD(1);
#endif
    }

    static bool isGlobalEnabled()
    {
#ifdef PLATFORM_SUPPORT_MPU
        return ((IP_MPU->CESR & (MPU_CESR_NRGD_MASK | MPU_CESR_VLD_MASK)) == 0x01U);
#else
        return false;
#endif
    }

    static void setDescriptor(size_t const slot, tDescriptor const& d)
    {
#ifdef PLATFORM_SUPPORT_MPU
        IP_MPU->RGD[slot].WORD0 = d.startAdress;
        IP_MPU->RGD[slot].WORD1 = d.endAdress;
        IP_MPU->RGD[slot].WORD2 = d.word2.R;
        IP_MPU->RGD[slot].WORD3 = d.word3.R;
#endif
    }

    template<uint8_t slot, uint8_t wordNumber, uint32_t word>
    static void setWordInDescriptor()
    {
#ifdef PLATFORM_SUPPORT_MPU
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
#endif
    }

    template<uint8_t wordNumber>
    static uint32_t getWordInDescriptor(uint8_t const slot)
    {
#ifdef PLATFORM_SUPPORT_MPU
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
#endif
        return 0U;
    }
};

template<
    Mpu::tReadAccess m7re,
    Mpu::tWriteAccess m7we,
    Mpu::tReadAccess m6re,
    Mpu::tWriteAccess m6we,
    Mpu::tReadAccess m5re,
    Mpu::tWriteAccess m5we,
    Mpu::tReadAccess m4re,
    Mpu::tWriteAccess m4we,
    Mpu::tAccessSupervisorMode m3sm,
    Mpu::tAccessUserMode m3um,
    Mpu::tAccessSupervisorMode m2sm,
    Mpu::tAccessUserMode m2um,
    Mpu::tProcessIdentifier m1pe,
    Mpu::tAccessSupervisorMode m1sm,
    Mpu::tAccessUserMode m1um,
    Mpu::tProcessIdentifier m0pe,
    Mpu::tAccessSupervisorMode m0sm,
    Mpu::tAccessUserMode m0um>
struct mpuWord2
{
    static uint32_t const word
        = (static_cast<uint32_t>(m7re) << 31) | (static_cast<uint32_t>(m7we) << 30)
          | (static_cast<uint32_t>(m6re) << 29) | (static_cast<uint32_t>(m6we) << 28)
          | (static_cast<uint32_t>(m5re) << 27) | (static_cast<uint32_t>(m5we) << 26)
          | (static_cast<uint32_t>(m4re) << 25) | (static_cast<uint32_t>(m4we) << 24)
          | (static_cast<uint32_t>(m3um) << 18) | (static_cast<uint32_t>(m2sm) << 15)
          | (static_cast<uint32_t>(m2um) << 12) | (static_cast<uint32_t>(m1pe) << 11)
          | (static_cast<uint32_t>(m1sm) << 9) | (static_cast<uint32_t>(m1um) << 6)
          | (static_cast<uint32_t>(m0pe) << 5) | (static_cast<uint32_t>(m0sm) << 3)
          | (static_cast<uint32_t>(m0um));
};

template<uint32_t pid, uint32_t pidmask, uint32_t vld>
struct mpuWord3
{
    static uint32_t const word = (static_cast<uint32_t>(pid) << 24)
                                 | (static_cast<uint32_t>(pidmask) << 16)
                                 | (static_cast<uint32_t>(vld));
};

} // namespace safety
