// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#include <adc/Adc.h>

namespace bios
{

Adc::Adc(AdcConfig const& config) : fConfig(config), fInitialized(false) {}

void Adc::enableClock()
{
#if defined(STM32G474xx)
    // STM32G4: ADC12 clock on AHB2
    RCC->AHB2ENR |= RCC_AHB2ENR_ADC12EN;
    uint32_t volatile dummy = RCC->AHB2ENR;
    (void)dummy;

    // Select system clock as ADC clock (ADCSEL = 01 in CCIPR1)
    // This gives maximum ADC clock speed
    RCC->CCIPR = (RCC->CCIPR & ~(3U << 28U)) | (1U << 28U);
#elif defined(STM32F413xx)
    // STM32F4: ADC1 clock on APB2
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    uint32_t volatile dummy = RCC->APB2ENR;
    (void)dummy;
#endif
}

void Adc::calibrate()
{
#if defined(STM32G474xx)
    ADC_TypeDef* adc = fConfig.peripheral;

    // Ensure ADC is disabled and voltage regulator is on
    adc->CR &= ~ADC_CR_ADEN;
    adc->CR &= ~ADC_CR_DEEPPWD; // Exit deep power-down
    adc->CR |= ADC_CR_ADVREGEN; // Enable voltage regulator

    // Wait for regulator startup (~20us)
    for (uint32_t volatile i = 0U; i < 10000U; i++)
    {
    }

    // Single-ended calibration
    adc->CR &= ~ADC_CR_ADCALDIF;
    adc->CR |= ADC_CR_ADCAL;
#if !defined(UNIT_TEST)
    while ((adc->CR & ADC_CR_ADCAL) != 0U)
    {
    }
#else
    adc->CR &= ~ADC_CR_ADCAL; // Simulate calibration complete
#endif
#elif defined(STM32F413xx)
    // STM32F4 ADC doesn't have a hardware calibration sequence
    // Just ensure ADC is off
    fConfig.peripheral->CR2 &= ~ADC_CR2_ADON;
#endif
}

void Adc::init()
{
    enableClock();
    calibrate();

    ADC_TypeDef* adc = fConfig.peripheral;

#if defined(STM32G474xx)
    // Set resolution
    adc->CFGR = (adc->CFGR & ~ADC_CFGR_RES) | (static_cast<uint32_t>(fConfig.resolution) << 3U);

    // Single conversion mode, software trigger, right-aligned
    adc->CFGR &= ~(ADC_CFGR_CONT | ADC_CFGR_EXTEN);
    adc->CFGR &= ~ADC_CFGR_ALIGN; // Right-aligned

    // Enable ADC
    adc->ISR |= ADC_ISR_ADRDY; // Clear ready flag
    adc->CR |= ADC_CR_ADEN;
    while ((adc->ISR & ADC_ISR_ADRDY) == 0U)
    {
    }
#elif defined(STM32F413xx)
    // Set resolution
    adc->CR1 = (adc->CR1 & ~ADC_CR1_RES) | (static_cast<uint32_t>(fConfig.resolution) << 24U);

    // Single conversion, software trigger, right-aligned
    adc->CR2 &= ~(ADC_CR2_CONT | ADC_CR2_ALIGN);

    // Prescaler: PCLK2/4 (common ADC register)
    ADC_Common_TypeDef* common = ADC123_COMMON;
    common->CCR = (common->CCR & ~ADC_CCR_ADCPRE) | (1U << 16U); // /4

    // Enable internal temp sensor and VREFINT
    common->CCR |= ADC_CCR_TSVREFE;

    // Enable ADC
    adc->CR2 |= ADC_CR2_ADON;
#endif

    fInitialized = true;
}

void Adc::configureChannel(uint8_t channel)
{
    ADC_TypeDef* adc = fConfig.peripheral;

#if defined(STM32G474xx)
    // Regular sequence: 1 conversion, channel in SQR1
    adc->SQR1 = (adc->SQR1 & ~(ADC_SQR1_L | ADC_SQR1_SQ1))
                | (static_cast<uint32_t>(channel) << 6U); // L=0 (1 conv), SQ1=channel

    // Sampling time
    if (channel < 10U)
    {
        uint32_t pos = channel * 3U;
        adc->SMPR1   = (adc->SMPR1 & ~(7U << pos))
                     | (static_cast<uint32_t>(fConfig.samplingTime) << pos);
    }
    else
    {
        uint32_t pos = (channel - 10U) * 3U;
        adc->SMPR2   = (adc->SMPR2 & ~(7U << pos))
                     | (static_cast<uint32_t>(fConfig.samplingTime) << pos);
    }
#elif defined(STM32F413xx)
    // Regular sequence: 1 conversion, channel in SQR3
    adc->SQR1 &= ~ADC_SQR1_L;                                        // L=0 (1 conv)
    adc->SQR3 = (adc->SQR3 & ~0x1FU) | (channel & 0x1FU);           // SQ1=channel

    // Sampling time (use SMPR2 for channels 0-9, SMPR1 for 10-18)
    if (channel < 10U)
    {
        uint32_t pos = channel * 3U;
        adc->SMPR2   = (adc->SMPR2 & ~(7U << pos))
                     | (static_cast<uint32_t>(fConfig.samplingTime) << pos);
    }
    else
    {
        uint32_t pos = (channel - 10U) * 3U;
        adc->SMPR1   = (adc->SMPR1 & ~(7U << pos))
                     | (static_cast<uint32_t>(fConfig.samplingTime) << pos);
    }
#endif
}

uint16_t Adc::startAndRead()
{
    ADC_TypeDef* adc = fConfig.peripheral;

#if defined(STM32G474xx)
    adc->ISR |= ADC_ISR_EOC; // Clear EOC
    adc->CR |= ADC_CR_ADSTART;
    while ((adc->ISR & ADC_ISR_EOC) == 0U)
    {
    }
    return static_cast<uint16_t>(adc->DR);
#elif defined(STM32F413xx)
    adc->SR &= ~ADC_SR_EOC;
    adc->CR2 |= ADC_CR2_SWSTART;
    while ((adc->SR & ADC_SR_EOC) == 0U)
    {
    }
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
