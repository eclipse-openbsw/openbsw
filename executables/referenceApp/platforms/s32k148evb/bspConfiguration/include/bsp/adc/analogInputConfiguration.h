// Copyright 2024 Accenture.

// IGNORE_INCLUDE_GUARD_CHECK

#if defined(BSP_ANALOGINPUT_PIN_CONFIGURATION) && (BSP_ANALOGINPUT_PIN_CONFIGURATION == 1)
/**
 * Configuration of analog signals
 *
 * Order must be the same as the order of the signal IDs defined below.
 *
 *      <ADC device>, <external input>, <pin>, <conversion channel>
 */
AnalogInput::tAnalogConfiguration const ADChannelCfg[AnalogInput::_AdcChannelMax] = {
    {ADC_0_CH, _ADCCHANNEL28, Io::EVAL_POTI_ADC, _ADCCHANNEL0},
    {ADC_0_CH, _ADCCHANNEL29, Io::EVAL_ADC, _ADCCHANNEL0},
};

#else
/**
 * Analog input signal IDs
 */
enum AnalogInputId
{
    AiEVAL_POTI_ADC,
    AiEval_ADC,

    // end marker
    _AdcChannelMax
};

#endif
