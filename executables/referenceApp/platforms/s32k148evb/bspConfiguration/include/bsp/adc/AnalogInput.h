// Copyright 2024 Accenture.

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
        CFG1 = ADC_CFG1_ADIV(0) | ADC_CFG1_MODE(1), // Clock divided by 1, 12-bit conversion mode
        CFG2 = ADC_CFG2_SMPLTS(41),                 // Sample time is 42 ADC clocks
        CV1  = 0,
        CV2  = 0,
        SC2  = ADC_SC2_ADTRG(0) | ADC_SC2_DMAEN(0), // SW trigger, DMA is off
        SC3  = ADC_SC3_CAL(0) | ADC_SC3_ADCO(0)
              | ADC_SC3_AVGE(0), // Calibration disabled, One conversion, HW average disabled
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
