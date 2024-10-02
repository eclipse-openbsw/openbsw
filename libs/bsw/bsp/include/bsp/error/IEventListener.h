// Copyright 2024 Accenture.

#ifndef GUARD_8F64FED2_77A8_43B6_99FE_8AED4251BBAC
#define GUARD_8F64FED2_77A8_43B6_99FE_8AED4251BBAC

#include <platform/estdint.h>

namespace bios
{
class IEventListener
{
public:
    static uint32_t const INVALID = 0xffffffffU;

    static uint32_t const ADCERROR          = 0x00000001U;
    static uint32_t const FLASHERROR        = 0x00000002U;
    static uint32_t const EEPERROR          = 0x00000004U;
    static uint32_t const TIMERERROR        = 0x00000008U;
    static uint32_t const PWMERROR          = 0x00000010U;
    static uint32_t const SPIERROR          = 0x00000020U;
    static uint32_t const CLOCKMONITORERROR = 0x00000040U;
    static uint32_t const WDERROR           = 0x00000080U;
    static uint32_t const DMAERROR          = 0x00000100U;
    static uint32_t const STANDBYWARNING    = 0x00000200U;
    static uint32_t const RAMERROR          = 0x00000400U;
    static uint32_t const SAFE_IO           = 0x00000800U;
    static uint32_t const SAFE_SPOC         = 0x00001000U;
    static uint32_t const SAFE_EMIOS1       = 0x00002000U;
    static uint32_t const OVER_TEMPERATURE  = 0x00004000U;

    static uint32_t const EVENT15                = 0x00008000U;
    static uint32_t const EVENT16                = 0x00010000U;
    static uint32_t const EVENT17                = 0x00020000U;
    static uint32_t const EVENT18                = 0x00040000U;
    static uint32_t const EVENT19                = 0x00080000U;
    static uint32_t const EVENT20                = 0x00100000U;
    static uint32_t const EVENT21                = 0x00200000U;
    static uint32_t const EVENT22                = 0x00400000U;
    static uint32_t const EVENT23                = 0x00800000U;
    static uint32_t const EVENT24                = 0x01000000U;
    static uint32_t const EVENT25                = 0x02000000U;
    static uint32_t const EVENT26                = 0x04000000U;
    static uint32_t const EVENT27                = 0x08000000U;
    static uint32_t const EVENT28                = 0x10000000U;
    static uint32_t const EVENT29                = 0x20000000U;
    static uint32_t const EVENT30                = 0x40000000U;
    static uint32_t const SYSTEM_UNLOCKEDWARNING = 0x80000000U;

    virtual void event(
        uint32_t event,
        bool active,
        uint32_t status0 = INVALID,
        uint32_t status1 = INVALID,
        uint32_t status2 = INVALID,
        uint32_t status3 = INVALID

        )
        = 0;

protected:
    IEventListener& operator=(IEventListener const&) = default;
};

} // namespace bios

#endif /* GUARD_8F64FED2_77A8_43B6_99FE_8AED4251BBAC */
