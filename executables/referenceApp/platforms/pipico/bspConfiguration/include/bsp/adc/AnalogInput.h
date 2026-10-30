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

#include "adc/Adc.h"

namespace bios
{
static uint8_t const ADC_0_CH = 0U;

/**
 * ADC configuration
 */
class Adc12BitConfiguration
{
public:
    enum
    {
        BASE_OFS = 0x40UL,
        OFS      = 0x0UL,
        USR_OFS  = 0,
        XOFS     = 0x30UL,
        YOFS     = 0x37UL,
        UG       = 0x4UL,
        CLPS_OFS = 0x0UL,
    };
};

typedef AdcInResolution<uint16_t, adc12BitResolutionMask, adcDataShift0> tAdc12BitConvertor;

typedef Adc<tAdc12BitConvertor, Adc12BitConfiguration> tAdc12Bit;

class AnalogInput
{
public:
    AnalogInput();

#undef BSP_ANALOGINPUT_PIN_CONFIGURATION
#include "bsp/adc/analogInputConfiguration.h"

    typedef struct
    {
        uint8_t adc;
        uint8_t phChannel;
        uint8_t pin;
        uint8_t slot;
    } tAnalogConfiguration;

    void init();
    void start();

    static bsp::BspReturnCode get(uint16_t channel, uint16_t& value);
    static bsp::BspReturnCode get(uint16_t channel, uint32_t& value);

    static char const* getName(uint16_t channel);

    void stop();

    void cyclic();

    static uint16_t const AdcCountAll = static_cast<uint16_t>(_AdcChannelMax);

private:
    static tAdc12Bit fAdc0;

    static tAnalogConfiguration const* fADChannelCfg;
};

} // namespace bios
