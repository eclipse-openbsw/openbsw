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

#include "adc/adcPhysicalName.h"
#include "adc/adcResolution.h"
#include "assert.h"
#include "bsp/Bsp.h"
#include "bsp/SystemTime.h"
#include "mcu/mcu.h"
#include "platform/estdint.h"

#include <etl/uncopyable.h>

namespace bios
{
/*
ADC TOTAL CONVERSION TIME = Sample Phase Time (set by SMPLTS + 1) + Hold
Phase (1 ADC Cycle) + Compare Phase Time (8-bit Mode = 20 ADC Cycles, 10-bit
Mode = 24 ADC Cycles, 12-bit Mode = 28 ADC Cycles) + Single or First continuous
time adder (5 ADC cycles + 5 bus clock cycles)
*/

struct ADC_Type
{};

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels = 16>
class Adc
: private AdcConfiguration
, public ::etl::uncopyable
{
public:
    Adc(ADC_Type& ADC) : fAdcInResolution(), fAdc(ADC), fIsStarted(false), fIsInitialized(false) {}

    bsp::BspReturnCode init();
    bsp::BspReturnCode initSleep();
    void enableChannel(uint8_t channel, uint8_t extInput, bool isrEn = false);
    bsp::BspReturnCode startInjection();
    bsp::BspReturnCode InjectionsReady(uint32_t timeout);
    bsp::BspReturnCode start();
    bsp::BspReturnCode stop();
    bsp::BspReturnCode getValue(uint8_t channel, uint16_t& value) const;
    bsp::BspReturnCode getValueSync(uint8_t channel, uint16_t& value) const;
    bsp::BspReturnCode safe();
    bsp::BspReturnCode dma(bool active) const;
    bsp::BspReturnCode row2Value(uint32_t r, uint16_t& v) const;

    AdcResolution fAdcInResolution;
    ADC_Type& fAdc;
    bool fIsStarted;
    bool fIsInitialized;
};

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
void Adc<AdcResolution, AdcConfiguration, maxChannels>::enableChannel(
    uint8_t channel, uint8_t extInput, bool isrEn)
{}

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode Adc<AdcResolution, AdcConfiguration, maxChannels>::init()
{
    return bsp::BSP_OK;
}

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode Adc<AdcResolution, AdcConfiguration, maxChannels>::initSleep()
{
    return bsp::BSP_OK;
}

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode Adc<AdcResolution, AdcConfiguration, maxChannels>::start()
{
    return bsp::BSP_OK;
}

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode Adc<AdcResolution, AdcConfiguration, maxChannels>::safe()
{
    bsp::BspReturnCode ret = bsp::BSP_OK;

    return ret;
}

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode Adc<AdcResolution, AdcConfiguration, maxChannels>::startInjection()
{
    return bsp::BSP_OK;
}

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode
Adc<AdcResolution, AdcConfiguration, maxChannels>::InjectionsReady(uint32_t timeout)
{
    return bsp::BSP_OK;
}

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode Adc<AdcResolution, AdcConfiguration, maxChannels>::stop()
{
    return bsp::BSP_OK;
}

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode
Adc<AdcResolution, AdcConfiguration, maxChannels>::getValue(uint8_t channel, uint16_t& value) const
{
    return bsp::BSP_OK;
}

// Injected conversion
template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode Adc<AdcResolution, AdcConfiguration, maxChannels>::getValueSync(
    uint8_t phChannel, uint16_t& value) const
{
    return bsp::BSP_OK;
}

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode Adc<AdcResolution, AdcConfiguration, maxChannels>::dma(bool active) const
{
    return bsp::BSP_OK;
}

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode
Adc<AdcResolution, AdcConfiguration, maxChannels>::row2Value(uint32_t row, uint16_t& value) const
{
    return bsp::BSP_OK;
}

} // namespace bios
