// Copyright 2024 Accenture.

#ifndef GUARD_88D66BA5_75C5_4F3D_BBCA_102D4305AA3F
#define GUARD_88D66BA5_75C5_4F3D_BBCA_102D4305AA3F

#include "adc/adcPhysicalName.h"
#include "adc/adcResolution.h"
#include "assert.h"
#include "bsp/Bsp.h"
#include "bsp/SystemTime.h"
#include "estd/uncopyable.h"
#include "mcu/mcu.h"
#include "platform/estdint.h"

namespace bios
{
/*
ADC TOTAL CONVERSION TIME = Sample Phase Time (set by SMPLTS + 1) + Hold
Phase (1 ADC Cycle) + Compare Phase Time (8-bit Mode = 20 ADC Cycles, 10-bit
Mode = 24 ADC Cycles, 12-bit Mode = 28 ADC Cycles) + Single or First continuous
time adder (5 ADC cycles + 5 bus clock cycles)
*/

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels = 16>
class Adc : private AdcConfiguration
{
    UNCOPYABLE(Adc);

public:
    Adc(ADC_Type& ADC) : fAdcInResolution(), fAdc(ADC), fIsStarted(false), fIsInitialized(false) {}

    bsp::BspReturnCode init();
    bsp::BspReturnCode initSleep();
    void enableChannel(uint8_t channel, uint8_t extInput, bool isrEn = false);
    bsp::BspReturnCode startInjection();
    bsp::BspReturnCode InjectionsReady(uint32_t timeout);
    bsp::BspReturnCode start();
    bsp::BspReturnCode stop();
    bsp::BspReturnCode getValue(uint8_t channel, uint16_t& value) const;
    bsp::BspReturnCode getValueSync(uint8_t channel, uint16_t& value) const;
    bsp::BspReturnCode safe();
    bsp::BspReturnCode dma(bool active) const;
    bsp::BspReturnCode row2Value(uint32_t r, uint16_t& v) const;

    AdcResolution fAdcInResolution;
    ADC_Type& fAdc;
    bool fIsStarted;
    bool fIsInitialized;
};

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
void Adc<AdcResolution, AdcConfiguration, maxChannels>::enableChannel(
    uint8_t channel, uint8_t extInput, bool isrEn)
{
    if (channel < maxChannels)
    {
        fAdc.SC1[channel] = ADC_SC1A_ADCH(extInput) + ADC_SC1A_AIEN(isrEn);
        if (0U == channel)
        {
            uint32_t timeout = 0;
            do
            {
                timeout++;
                if (timeout > 0xffffff00UL)
                {
                    break;
                }
            } while ((fAdc.SC1[0] & ADC_SC1A_COCO_MASK) == 0);
            ESR_UNUSED uint32_t a = fAdcInResolution(fAdc.R[0]);
        }
    }
}

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode Adc<AdcResolution, AdcConfiguration, maxChannels>::init()
{
    fAdc.CFG1  = AdcConfiguration::CFG1;
    fAdc.CFG2  = AdcConfiguration::CFG2;
    fAdc.CV[0] = AdcConfiguration::CV1;
    fAdc.CV[1] = AdcConfiguration::CV2;

    fAdc.BASE_OFS = AdcConfiguration::BASE_OFS;
    fAdc.OFS      = AdcConfiguration::OFS;
    fAdc.USR_OFS  = AdcConfiguration::USR_OFS;
    fAdc.XOFS     = AdcConfiguration::XOFS;
    fAdc.YOFS     = AdcConfiguration::YOFS;

    fAdc.CLPS_OFS = AdcConfiguration::CLPS_OFS;

    // Disable all channels
    for (uint8_t i = 0; i < maxChannels; i++)
    {
        fAdc.SC1[i] = ADC_SC1A_ADCH_MASK;
    }

    fAdc.CLP3 = 0;
    fAdc.CLP2 = 0;
    fAdc.CLP1 = 0;

    fAdc.CLP0                      = 0;
    fAdc.CLPX                      = 0;
    fAdc.CLP9                      = 0;
    ESR_UNUSED volatile uint32_t a = fAdc.R[0];
    fAdc.SC2                       = 0;
    // Start calibration, HW average function enabled, 32 samples averaged
    fAdc.SC3                       = ADC_SC3_CAL_MASK | ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(3);

    uint32_t timeout = 0;
    do
    {
        timeout++;
        if (timeout > 0xffffff00UL)
        {
            break;
        }
    } while ((fAdc.SC1[0] & ADC_SC1A_COCO_MASK) == 0);
    a = fAdc.R[0];

    fAdc.UG  = AdcConfiguration::UG;
    fAdc.SC2 = AdcConfiguration::SC2;
    fAdc.SC3 = AdcConfiguration::SC3;

    fIsStarted     = false;
    fIsInitialized = true;

    return bsp::BSP_OK;
}

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode Adc<AdcResolution, AdcConfiguration, maxChannels>::initSleep()
{
    fAdc.CFG1  = AdcConfiguration::CFG1;
    fAdc.CFG2  = AdcConfiguration::CFG2;
    fAdc.CV[0] = AdcConfiguration::CV1;
    fAdc.CV[1] = AdcConfiguration::CV2;

    fAdc.BASE_OFS = AdcConfiguration::BASE_OFS;
    fAdc.OFS      = AdcConfiguration::OFS;
    fAdc.USR_OFS  = AdcConfiguration::USR_OFS;
    fAdc.XOFS     = AdcConfiguration::XOFS;
    fAdc.YOFS     = AdcConfiguration::YOFS;

    fAdc.CLPS_OFS = AdcConfiguration::CLPS_OFS;

    ESR_UNUSED volatile uint32_t a = fAdc.R[0];

    fAdc.UG  = AdcConfiguration::UG;
    fAdc.SC2 = AdcConfiguration::SC2;
    fAdc.SC3 = AdcConfiguration::SC3;

    fIsStarted     = true;
    fIsInitialized = true;

    return bsp::BSP_OK;
}

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode Adc<AdcResolution, AdcConfiguration, maxChannels>::start()
{
    if ((false == fIsStarted) && (true == fIsInitialized))
    {
        fIsStarted = true;
    }
    return fIsStarted ? bsp::BSP_OK : bsp::BSP_ERROR;
}

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode Adc<AdcResolution, AdcConfiguration, maxChannels>::safe()
{
    bsp::BspReturnCode ret = bsp::BSP_OK;

    return ret;
}

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode Adc<AdcResolution, AdcConfiguration, maxChannels>::startInjection()
{
    if (true == fIsInitialized) {}
    return fIsInitialized ? bsp::BSP_OK : bsp::BSP_ERROR;
}

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode
Adc<AdcResolution, AdcConfiguration, maxChannels>::InjectionsReady(uint32_t timeout)
{
    bool mask0Ready = false;
    bool mask1Ready = false;
    bool mask2Ready = false;

    return (mask0Ready && mask1Ready && mask2Ready) ? bsp::BSP_OK : bsp::BSP_ERROR;
}

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode Adc<AdcResolution, AdcConfiguration, maxChannels>::stop()
{
    fIsStarted     = false;
    fIsInitialized = false;
    for (uint8_t i = 0; i < maxChannels; i++)
    {
        fAdc.SC1[i] = ADC_SC1A_ADCH_MASK;
    }
    return bsp::BSP_OK;
}

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode
Adc<AdcResolution, AdcConfiguration, maxChannels>::getValue(uint8_t channel, uint16_t& value) const
{
    if ((false == fIsInitialized) || (false == fIsStarted))
    {
        return bsp::BSP_ERROR;
    }

    if (channel < maxChannels)
    {
        value = fAdcInResolution(fAdc.R[channel]);
        return bsp::BSP_OK;
    }
    else
    {
        return bsp::BSP_ERROR;
    }
}

// Injected conversion
template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode Adc<AdcResolution, AdcConfiguration, maxChannels>::getValueSync(
    uint8_t phChannel, uint16_t& value) const
{
    if ((false == fIsInitialized) || (false == fIsStarted))
    {
        return bsp::BSP_ERROR;
    }
    if (((fAdc.SC2 & ADC_SC2_ADTRG(1)) == 0)     // Software trigger selected
        && ((fAdc.SC2 & ADC_SC2_DMAEN(1)) == 0)) // DMA disabled
    {
        ESR_UNUSED volatile uint32_t a = fAdc.R[0];
        fAdc.SC1[0]                    = ADC_SC1A_ADCH(phChannel);

        uint32_t timeout = 0;
        do
        {
            timeout++;
            if (timeout > 0xffffff00UL)
            {
                return bsp::BSP_ERROR;
            }
        } while ((fAdc.SC1[0] & ADC_SC1A_COCO_MASK) == 0);
        value = fAdcInResolution(fAdc.R[0]);

        return bsp::BSP_OK;
    }
    else
    {
        return bsp::BSP_ERROR;
    }
}

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode Adc<AdcResolution, AdcConfiguration, maxChannels>::dma(bool active) const
{
    if ((false == fIsInitialized) || (false == fIsStarted))
    {
        return bsp::BSP_ERROR;
    }
    ESR_UNUSED volatile uint32_t a = fAdc.R[0];
    if (true == active)
    {
        fAdc.SC2 = fAdc.SC2 | ADC_SC2_DMAEN(1);
    }
    else
    {
        fAdc.SC2 = fAdc.SC2 & ADC_SC2_DMAEN(0);
    }
    return bsp::BSP_OK;
}

template<typename AdcResolution, typename AdcConfiguration, uint8_t maxChannels>
bsp::BspReturnCode
Adc<AdcResolution, AdcConfiguration, maxChannels>::row2Value(uint32_t row, uint16_t& value) const
{
    if ((false == fIsInitialized) || (false == fIsStarted))
    {
        return bsp::BSP_ERROR;
    }
    value = fAdcInResolution(row);
    return bsp::BSP_OK;
}

} // namespace bios

#endif /* GUARD_88D66BA5_75C5_4F3D_BBCA_102D4305AA3F */
