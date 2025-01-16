// Copyright 2024 Accenture.

#include "bsp/adc/AnalogInput.h"

#include "bsp/adc/AnalogInputScale.h"
#include "io/Io.h"

#ifdef BSP_ANALOGINPUT_PIN_CONFIGURATION
#undef BSP_ANALOGINPUT_PIN_CONFIGURATION
#endif
#define BSP_ANALOGINPUT_PIN_CONFIGURATION 1

namespace bios
{

#include "bsp/adc/analogInputConfiguration.h"
#include "bsp/adc/analogInputConfigurationStrings.h"
#include "bsp/adc/analogInputScaleConfiguration.h"

tAdc12Bit AnalogInput::fAdc0(*ADC0);

AnalogInput::tAnalogConfiguration const* AnalogInput::fADChannelCfg = 0;

AnalogInput::AnalogInput() { fADChannelCfg = &ADChannelCfg[0]; }

void AnalogInput::init()
{
    AnalogInputScale::init(
        static_cast<uint8_t>(
            sizeof(analogInputScaleConfiguration) / sizeof(AnalogInputScale::scale)),
        &analogInputScaleConfiguration[0]);

    SIM->ADCOPT  = 0U;
    SIM->CHIPCTL = SIM->CHIPCTL & ~SIM_CHIPCTL_PDB_BB_SEL_MASK;

    (void)fAdc0.init();

    for (uint16_t i = 0; i < static_cast<uint16_t>(_AdcChannelMax); ++i)
    {
        if (fADChannelCfg[i].adc == ADC_0_CH)
        {
            (void)Io::setDefaultConfiguration(fADChannelCfg[i].pin);
            fAdc0.enableChannel(fADChannelCfg[i].slot, fADChannelCfg[i].phChannel);
        }
        else
        {
            // nothing to do
        }
    }
}

void AnalogInput::start() { (void)fAdc0.start(); }

bsp::BspReturnCode AnalogInput::get(uint16_t channel, uint16_t& value)
{
    bsp::BspReturnCode ret = bsp::BSP_ERROR;

    if (fADChannelCfg[channel].adc == ADC_0_CH)
    {
        ret = fAdc0.getValueSync(fADChannelCfg[channel].phChannel, value);
    }

    return ret;
}

bsp::BspReturnCode AnalogInput::get(uint16_t const channel, uint32_t& value)
{
    uint16_t value16             = 0U;
    bsp::BspReturnCode const ret = get(channel, value16);
    value                        = static_cast<uint32_t>(value16);
    return ret;
}

void AnalogInput::cyclic() {}

void AnalogInput::stop() { (void)fAdc0.stop(); }

char const* AnalogInput::getName(uint16_t const channel)
{
    if (channel < sizeof(adcChannelsNamingStrings) / (sizeof(char*)))
    {
        return adcChannelsNamingStrings[channel];
    }
    else
    {
        return ("??? ->");
    }
}

} // namespace bios
