// Copyright 2024 Accenture.

#ifndef GUARD_03F862CF_BF0D_49C9_8099_4A303BD1E08F
#define GUARD_03F862CF_BF0D_49C9_8099_4A303BD1E08F

#include "platform/estdint.h"

namespace bios
{
/**
 * Variants
 */
enum tAdcDataResolution
{
    adcDataShift0,
    adcDataShift2 = 2,
    adcDataShift4 = 4
};

enum tAdcDataMask
{
    adc8BitResolutionMask  = 0xff,
    adc10BitResolutionMask = 0x3ff,
    adc12BitResolutionMask = 0xfff
};

template<typename RT, tAdcDataMask resolutionMask, tAdcDataResolution resolution>
class AdcInResolution
{
public:
    using retType = RT;

    enum convertorResolution
    {
        convertResolutionMask = resolutionMask,
        convertShiftParam     = resolution
    };

    retType operator()(uint32_t rawValue) const
    {
        retType ret = (rawValue & (convertResolutionMask)) >> resolution;
        return ret;
    }
};

} // namespace bios

#endif /* GUARD_03F862CF_BF0D_49C9_8099_4A303BD1E08F */
