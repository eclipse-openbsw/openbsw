/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <mcu/mcu.h>
#include <stdint.h>

namespace bios
{

enum class AdcResolution : uint8_t
{
    BITS_12 = 0U,
    BITS_10 = 1U,
    BITS_8  = 2U,
    BITS_6  = 3U
};

struct AdcConfig
{
    ADC_TypeDef* peripheral;
    AdcResolution resolution;
    uint8_t samplingTime; // SMPR code (0-7)
};

class Adc
{
public:
    explicit Adc(AdcConfig const& config);

    void init();
    uint16_t readChannel(uint8_t channel);
    uint16_t readTemperature();
    uint16_t readVrefint();

private:
    AdcConfig const fConfig;
    bool fInitialized;

    void enableClock();
    void calibrate();
    void configureChannel(uint8_t channel);
    uint16_t startAndRead();
};

} // namespace bios
