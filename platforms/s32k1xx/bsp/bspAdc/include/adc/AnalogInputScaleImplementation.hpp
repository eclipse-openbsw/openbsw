// Copyright 2024 Accenture.

#pragma once

#include "bsp/adc/AnalogInput.h"

template<typename T1, typename T2>
typename AnalogInputScaleImplementation<T1, T2>::scale*
    AnalogInputScaleImplementation<T1, T2>::fScaleCfg
    = nullptr;
template<typename T1, typename T2>
uint16_t AnalogInputScaleImplementation<T1, T2>::fScaleCfgSize = 0;

template<typename T1, typename T2>
void AnalogInputScaleImplementation<T1, T2>::init(
    uint16_t scaleConfigSize, AnalogInputScaleImplementation<T1, T2>::scale const* scaleConfig)
{
    fScaleCfg     = (scale*)scaleConfig;
    fScaleCfgSize = scaleConfigSize;
}

template<typename T1, typename T2>
bsp::BspReturnCode
AnalogInputScaleImplementation<T1, T2>::get(uint16_t const channel, scaleReturnType& value)
{
    bsp::BspReturnCode retCode = bsp::BSP_ERROR;
    if ((nullptr != fScaleCfg) && (0 != fScaleCfgSize))
    {
        if (channel < fScaleCfgSize)
        {
            scaleReturnType adcValue;
            retCode = ::bios::AnalogInput::get(channel, adcValue);
            if (retCode == bsp::BSP_OK)
            {
                scale fun = fScaleCfg[channel];
                value     = (fun)(adcValue);
                retCode   = bsp::BSP_OK;
            }
        }
    }
    return retCode;
}

template<typename T1, typename T2>
bsp::BspReturnCode AnalogInputScaleImplementation<T1, T2>::calc(
    uint16_t channel, scaleParameterType adcValue, scaleReturnType& value)
{
    bsp::BspReturnCode retCode = bsp::BSP_ERROR;
    if ((0 != fScaleCfg) && (0 != fScaleCfgSize))
    {
        if (channel < fScaleCfgSize)
        {
            scale fun = fScaleCfg[channel];
            value     = (fun)(adcValue);
            retCode   = bsp::BSP_OK;
        }
    }
    return retCode;
}

