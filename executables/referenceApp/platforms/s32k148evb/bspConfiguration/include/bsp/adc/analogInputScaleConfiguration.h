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

#include "bsp/adc/AnalogInputScale.h"

uint32_t const precisionFactor                                = 1000U;
uint32_t const _5_0Volt12BitResolution                        = 5000U * precisionFactor / 4095U;
AnalogInputScale::scale const analogInputScaleConfiguration[] = {
    // clang-format off
    &scaleFunction<uint32_t, _5_0Volt12BitResolution, precisionFactor, 0>, // AiEVAL_POTI_ADC
    &scaleFunction<uint32_t, _5_0Volt12BitResolution, precisionFactor, 0>, //AiEval_ADC
    &scaleFunction<uint32_t, _5_0Volt12BitResolution, precisionFactor, 0>, //Ai_BANDGAP
    &scaleFunction<uint32_t, _5_0Volt12BitResolution, precisionFactor, 0>, //Ai_ADCINTERNAL0
    // clang-format on
};
