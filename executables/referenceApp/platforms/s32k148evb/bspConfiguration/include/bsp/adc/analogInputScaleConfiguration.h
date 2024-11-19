// Copyright 2024 Accenture.

#ifndef GUARD_2A3E93D6_DA09_4054_9029_E6233102C5AD
#define GUARD_2A3E93D6_DA09_4054_9029_E6233102C5AD

#include "bsp/adc/AnalogInputScale.h"

uint32_t const precisionFactor                                = 1000U;
uint32_t const _5_0Volt12BitResolution                        = 5000U * precisionFactor / 4095U;
AnalogInputScale::scale const analogInputScaleConfiguration[] = {
    // clang-format off
    &scaleFunction<uint32_t, _5_0Volt12BitResolution, precisionFactor, 0>, // AiEVAL_POTI_ADC
    &scaleFunction<uint32_t, _5_0Volt12BitResolution, precisionFactor, 0>, //AiEval_ADC
    // clang-format on
};

#endif // GUARD_2A3E93D6_DA09_4054_9029_E6233102C5AD
