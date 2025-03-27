// Copyright 2024 Accenture.

#pragma once

#include "adcScale.h"
#include "bsp/adc/AnalogInput.h"

namespace bios
{

template<typename retType, typename parameterType>
class AnalogInputScaleImplementation
{
public:
    using scaleReturnType    = retType;
    using scaleParameterType = parameterType;
    using scale              = scaleReturnType (*)(scaleParameterType);

    static void init(uint16_t scaleConfigSize, scale const* scaleCfg);

    /**
     * see API Analog Input
     */
    static bsp::BspReturnCode get(uint16_t channel, scaleReturnType& value);
    /**
     * common conversion function for exp:threshold calculation
     */
    static bsp::BspReturnCode
    calc(uint16_t channel, scaleParameterType adcValue, scaleReturnType& value);

private:
    static scale* fScaleCfg;
    static uint16_t fScaleCfgSize;
};

#include "adc/AnalogInputScaleImplementation.hpp"

} // namespace bios
