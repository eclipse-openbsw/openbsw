/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <adc/Adc.h>

namespace bios
{

Adc::Adc(AdcConfig const& config) : fConfig(config), fInitialized(false) {}

void Adc::enableClock()
{
#if defined(STM32G474xx)
    RCC->AHB2ENR |= RCC_AHB2ENR_ADC12EN;
    uint32_t volatile dummy = RCC->AHB2ENR;
    (void)dummy;

    // Select system clock as ADC clock (ADCSEL = 01 in CCIPR)
    RCC->CCIPR = (RCC->CCIPR & ~(3U << 28U)) | (1U << 28U);
#elif defined(STM32F413xx)
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    uint32_t volatile dummy = RCC->APB2ENR;
    (void)dummy;
#endif
}

void Adc::calibrate()
{
#if defined(STM32G474xx)
    ADC_TypeDef* adc = fConfig.peripheral;

    adc->CR &= ~ADC_CR_ADEN;
    adc->CR &= ~ADC_CR_DEEPPWD;
    adc->CR |= ADC_CR_ADVREGEN;

    // Wait for regulator startup (~20us)
    for (uint32_t volatile i = 0U; i < 10000U; i++) {}

    adc->CR &= ~ADC_CR_ADCALDIF;
    adc->CR |= ADC_CR_ADCAL;
#if !defined(UNIT_TEST)
    while ((adc->CR & ADC_CR_ADCAL) != 0U) {}
#else
    adc->CR &= ~ADC_CR_ADCAL; // Simulate calibration complete
#endif
#elif defined(STM32F413xx)
    // STM32F4 ADC has no hardware calibration sequence
    fConfig.peripheral->CR2 &= ~ADC_CR2_ADON;
#endif
}

void Adc::init()
{
    enableClock();
    calibrate();

    ADC_TypeDef* adc = fConfig.peripheral;

#if defined(STM32G474xx)
    adc->CFGR = (adc->CFGR & ~ADC_CFGR_RES) | (static_cast<uint32_t>(fConfig.resolution) << 3U);
    adc->CFGR &= ~(ADC_CFGR_CONT | ADC_CFGR_EXTEN);
    adc->CFGR &= ~ADC_CFGR_ALIGN;

    adc->ISR |= ADC_ISR_ADRDY; // Write-1-to-clear ready flag
    adc->CR |= ADC_CR_ADEN;
    while ((adc->ISR & ADC_ISR_ADRDY) == 0U) {}
#elif defined(STM32F413xx)
    adc->CR1 = (adc->CR1 & ~ADC_CR1_RES) | (static_cast<uint32_t>(fConfig.resolution) << 24U);
    adc->CR2 &= ~(ADC_CR2_CONT | ADC_CR2_ALIGN);

    ADC_Common_TypeDef* common = ADC123_COMMON;
    common->CCR                = (common->CCR & ~ADC_CCR_ADCPRE) | (1U << 16U); // PCLK2/4
    common->CCR |= ADC_CCR_TSVREFE;

    adc->CR2 |= ADC_CR2_ADON;
#endif

    fInitialized = true;
}

void Adc::configureChannel(uint8_t channel)
{
    ADC_TypeDef* adc = fConfig.peripheral;

#if defined(STM32G474xx)
    adc->SQR1 = (adc->SQR1 & ~(ADC_SQR1_L | ADC_SQR1_SQ1))
                | (static_cast<uint32_t>(channel) << 6U); // L=0 (1 conv), SQ1=channel

    if (channel < 10U)
    {
        uint32_t pos = channel * 3U;
        adc->SMPR1
            = (adc->SMPR1 & ~(7U << pos)) | (static_cast<uint32_t>(fConfig.samplingTime) << pos);
    }
    else
    {
        uint32_t pos = (channel - 10U) * 3U;
        adc->SMPR2
            = (adc->SMPR2 & ~(7U << pos)) | (static_cast<uint32_t>(fConfig.samplingTime) << pos);
    }
#elif defined(STM32F413xx)
    adc->SQR1 &= ~ADC_SQR1_L;                             // L=0 (1 conv)
    adc->SQR3 = (adc->SQR3 & ~0x1FU) | (channel & 0x1FU); // SQ1=channel

    // F4: SMPR2 covers channels 0-9, SMPR1 covers 10-18 (reversed vs G4)
    if (channel < 10U)
    {
        uint32_t pos = channel * 3U;
        adc->SMPR2
            = (adc->SMPR2 & ~(7U << pos)) | (static_cast<uint32_t>(fConfig.samplingTime) << pos);
    }
    else
    {
        uint32_t pos = (channel - 10U) * 3U;
        adc->SMPR1
            = (adc->SMPR1 & ~(7U << pos)) | (static_cast<uint32_t>(fConfig.samplingTime) << pos);
    }
#endif
}

uint16_t Adc::startAndRead()
{
    ADC_TypeDef* adc = fConfig.peripheral;

#if defined(STM32G474xx)
    adc->ISR |= ADC_ISR_EOC; // Write-1-to-clear EOC
    adc->CR |= ADC_CR_ADSTART;
    while ((adc->ISR & ADC_ISR_EOC) == 0U) {}
    return static_cast<uint16_t>(adc->DR);
#elif defined(STM32F413xx)
    adc->SR &= ~ADC_SR_EOC;
    adc->CR2 |= ADC_CR2_SWSTART;
    while ((adc->SR & ADC_SR_EOC) == 0U) {}
    return static_cast<uint16_t>(adc->DR);
#else
    return 0U;
#endif
}

uint16_t Adc::readChannel(uint8_t channel)
{
    if (!fInitialized)
    {
        return 0U;
    }
    configureChannel(channel);
    return startAndRead();
}

uint16_t Adc::readTemperature()
{
#if defined(STM32G474xx)
    return readChannel(16U); // VSENSE on ADC1 ch16
#elif defined(STM32F413xx)
    return readChannel(18U); // VSENSE on ADC1 ch18
#else
    return 0U;
#endif
}

uint16_t Adc::readVrefint()
{
#if defined(STM32G474xx)
    return readChannel(18U); // VREFINT on ADC1 ch18
#elif defined(STM32F413xx)
    return readChannel(17U); // VREFINT on ADC1 ch17
#else
    return 0U;
#endif
}

} // namespace bios
