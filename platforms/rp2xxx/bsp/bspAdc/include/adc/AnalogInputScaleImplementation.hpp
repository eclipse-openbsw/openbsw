/********************************************************************************
 * Copyright (c) 2024 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
    return retCode;
}

template<typename T1, typename T2>
bsp::BspReturnCode AnalogInputScaleImplementation<T1, T2>::calc(
    uint16_t channel, scaleParameterType adcValue, scaleReturnType& value)
{
    bsp::BspReturnCode retCode = bsp::BSP_ERROR;
    return retCode;
}
