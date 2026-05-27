/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
