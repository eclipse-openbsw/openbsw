// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   Adc.h
 * \brief  Low-level ADC driver for STM32 Cortex-M4 platforms.
 *
 * Supports single-conversion mode on ADC1 for both STM32F4 and STM32G4.
 * Provides channel selection, resolution configuration, and blocking read.
 *
 * STM32F413ZH: ADC1, 12-bit, 16 channels, PCLK2/4 clock
 * STM32G474RE: ADC1/ADC2, 12-bit, hardware oversampling, AHB clock
 */

#pragma once

#include <mcu/mcu.h>
#include <stdint.h>

namespace bios
{

enum class AdcResolution : uint8_t
{
    BITS_12 = 0U, ///< 12-bit (4096 levels)
    BITS_10 = 1U, ///< 10-bit (1024 levels)
    BITS_8  = 2U, ///< 8-bit (256 levels)
    BITS_6  = 3U  ///< 6-bit (64 levels)
};

struct AdcConfig
{
    ADC_TypeDef* peripheral;    ///< ADC1 or ADC2
    AdcResolution resolution;   ///< Conversion resolution
    uint8_t samplingTime;       ///< Sampling time code (0-7)
};

/**
 * \brief Low-level ADC driver for STM32.
 */
class Adc
{
public:
    explicit Adc(AdcConfig const& config);

    /**
     * \brief Initialize the ADC peripheral (clock, calibration, ready).
     */
    void init();

    /**
     * \brief Perform a single blocking conversion on the given channel.
     * \param channel  ADC channel number (0-18 depending on device).
     * \return Conversion result (right-aligned).
     */
    uint16_t readChannel(uint8_t channel);

    /**
     * \brief Read the internal temperature sensor (channel 16 on F4, ch 16 on G4).
     * \return Raw ADC value. Use device-specific calibration to convert to degrees.
     */
    uint16_t readTemperature();

    /**
     * \brief Read the internal voltage reference (VREFINT).
     * \return Raw ADC value.
     */
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
