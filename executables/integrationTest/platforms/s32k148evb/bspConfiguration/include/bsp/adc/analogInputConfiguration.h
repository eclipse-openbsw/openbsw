/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
    {ADC_0_CH, _BANDGAP, Io::PORT_UNAVAILABLE, _ADCCHANNEL13},
    {ADC_0_CH, _ADCINTERNAL0, Io::PORT_UNAVAILABLE, _ADCCHANNEL14},
};

#else
/**
 * Analog input signal IDs
 */
enum AnalogInputId
{
    AiEVAL_POTI_ADC,
    AiEval_ADC,
    Ai_BANDGAP,
    Ai_ADCINTERNAL0,

    // end marker
    _AdcChannelMax
};

#endif
