// Copyright 2024 Accenture.

#pragma once

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

