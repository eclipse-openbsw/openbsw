// Copyright 2024 Accenture.

#include "clock/clockConfig.h"

#include <mcu/mcu.h>
// clang-format off
// needs to be included after mcu.h
#include <bsp/clock/boardClock.h>
// clang-format on

#include <platform/estdint.h>

#ifdef SCG_SPLLCSR_SPLLEN_MASK
#define PLL_AVAILABLE 1
#else
#define PLL_AVAILABLE 0
#endif

// internal primitives
namespace
{
void disablePeripheralClocks(void)
{
    PCC->PCCn[PCC_FTFC_INDEX]     = 0U;
    PCC->PCCn[PCC_DMAMUX_INDEX]   = 0U;
    PCC->PCCn[PCC_FlexCAN0_INDEX] = 0U;
#ifdef PCC_FlexCAN1_INDEX
    PCC->PCCn[PCC_FlexCAN1_INDEX] = 0U;
#endif
#ifdef PCC_FTM3_INDEX
    PCC->PCCn[PCC_FTM3_INDEX] = 0U;
#endif
#ifdef PCC_ADC1_INDEX
    PCC->PCCn[PCC_ADC1_INDEX] = 0U;
#endif
#ifdef PCC_FlexCAN2_INDEX
    PCC->PCCn[PCC_FlexCAN2_INDEX] = 0U;
#endif
    PCC->PCCn[PCC_LPSPI0_INDEX] = 0U;
#ifdef PCC_LPSPI1_INDEX
    PCC->PCCn[PCC_LPSPI1_INDEX] = 0U;
#endif
#ifdef PCC_LPSPI2_INDEX
    PCC->PCCn[PCC_LPSPI2_INDEX] = 0U;
#endif
#ifdef PCC_PDB1_INDEX
    PCC->PCCn[PCC_PDB1_INDEX] = 0U;
#endif
    PCC->PCCn[PCC_CRC_INDEX]  = 0U;
    PCC->PCCn[PCC_PDB0_INDEX] = 0U;
    PCC->PCCn[PCC_LPIT_INDEX] = 0U;
    PCC->PCCn[PCC_FTM0_INDEX] = 0U;
    PCC->PCCn[PCC_FTM1_INDEX] = 0U;
#ifdef PCC_FTM2_INDEX
    PCC->PCCn[PCC_FTM2_INDEX] = 0U;
#endif
    PCC->PCCn[PCC_ADC0_INDEX]   = 0U;
    PCC->PCCn[PCC_RTC_INDEX]    = 0U;
    PCC->PCCn[PCC_LPTMR0_INDEX] = 0U;
    PCC->PCCn[PCC_PORTA_INDEX]  = 0U;
    PCC->PCCn[PCC_PORTB_INDEX]  = 0U;
    PCC->PCCn[PCC_PORTC_INDEX]  = 0U;
    PCC->PCCn[PCC_PORTD_INDEX]  = 0U;
    PCC->PCCn[PCC_PORTE_INDEX]  = 0U;
    PCC->PCCn[PCC_FlexIO_INDEX] = 0U;
#ifdef PCC_EWM_INDEX
    PCC->PCCn[PCC_EWM_INDEX] = 0U;
#endif
    PCC->PCCn[PCC_LPI2C0_INDEX]  = 0U;
    PCC->PCCn[PCC_LPUART0_INDEX] = 0U;
    PCC->PCCn[PCC_LPUART1_INDEX] = 0U;
#ifdef PCC_LPUART2_INDEX
    PCC->PCCn[PCC_LPUART2_INDEX] = 0U;
#endif
#ifdef PCC_FTM4_INDEX
    PCC->PCCn[PCC_FTM4_INDEX] = 0U;
#endif
#ifdef PCC_FTM5_INDEX
    PCC->PCCn[PCC_FTM5_INDEX] = 0U;
#endif
    PCC->PCCn[PCC_CMP0_INDEX] = 0U;
#ifdef PCC_CMU0_INDEX
    PCC->PCCn[PCC_CMU0_INDEX] = 0U;
#endif
#ifdef PCC_CMU1_INDEX
    PCC->PCCn[PCC_CMU1_INDEX] = 0U;
#endif
#ifdef PCC_ENET_INDEX
    PCC->PCCn[PCC_ENET_INDEX] = 0U;
#endif
}

void enablePeripheralClocks(void)
{
    PCC->PCCn[PCC_FTFC_INDEX]     = 0xC0000000U;
    PCC->PCCn[PCC_DMAMUX_INDEX]   = 0xC0000000U;
    PCC->PCCn[PCC_FlexCAN0_INDEX] = 0xC0000000U;
#ifdef PCC_FlexCAN1_INDEX
    PCC->PCCn[PCC_FlexCAN1_INDEX] = 0xC0000000U;
#endif
#ifdef PCC_FTM3_INDEX
    PCC->PCCn[PCC_FTM3_INDEX] = CLOCK_SOURCE_FTM3;
    PCC->PCCn[PCC_FTM3_INDEX] = PCC->PCCn[PCC_FTM3_INDEX] | 0xC0000000U;
#endif
#ifdef PCC_ADC1_INDEX
    PCC->PCCn[PCC_ADC1_INDEX] = CLOCK_SOURCE_ADC1;
    PCC->PCCn[PCC_ADC1_INDEX] = PCC->PCCn[PCC_ADC1_INDEX] | 0xC0000000U;
#endif
#ifdef PCC_FlexCAN2_INDEX
    PCC->PCCn[PCC_FlexCAN2_INDEX] = 0xC0000000U;
#endif
    PCC->PCCn[PCC_LPSPI0_INDEX] = CLOCK_SOURCE_LPSPI0;
    PCC->PCCn[PCC_LPSPI0_INDEX] = PCC->PCCn[PCC_LPSPI0_INDEX] | 0xC0000000U;
#ifdef PCC_LPSPI1_INDEX
    PCC->PCCn[PCC_LPSPI1_INDEX] = CLOCK_SOURCE_LPSPI1;
    PCC->PCCn[PCC_LPSPI1_INDEX] = PCC->PCCn[PCC_LPSPI1_INDEX] | 0xC0000000U;
#endif
#ifdef PCC_LPSPI2_INDEX
    PCC->PCCn[PCC_LPSPI2_INDEX] = CLOCK_SOURCE_LPSPI2;
    PCC->PCCn[PCC_LPSPI2_INDEX] = PCC->PCCn[PCC_LPSPI2_INDEX] | 0xC0000000U;
#endif
#ifdef PCC_PDB1_INDEX
    PCC->PCCn[PCC_PDB1_INDEX] = 0xC0000000U;
#endif
    PCC->PCCn[PCC_CRC_INDEX]  = 0xC0000000U;
    PCC->PCCn[PCC_PDB0_INDEX] = 0xC0000000U;
    PCC->PCCn[PCC_LPIT_INDEX] = CLOCK_SOURCE_LPIT;
    PCC->PCCn[PCC_LPIT_INDEX] = PCC->PCCn[PCC_LPIT_INDEX] | 0xC0000000U;
    PCC->PCCn[PCC_FTM0_INDEX] = CLOCK_SOURCE_FTM0;
    PCC->PCCn[PCC_FTM0_INDEX] = PCC->PCCn[PCC_FTM0_INDEX] | 0xC0000000U;
    PCC->PCCn[PCC_FTM1_INDEX] = CLOCK_SOURCE_FTM1;
    PCC->PCCn[PCC_FTM1_INDEX] = PCC->PCCn[PCC_FTM1_INDEX] | 0xC0000000U;
#ifdef PCC_FTM2_INDEX
    PCC->PCCn[PCC_FTM2_INDEX] = CLOCK_SOURCE_FTM2;
    PCC->PCCn[PCC_FTM2_INDEX] = PCC->PCCn[PCC_FTM2_INDEX] | 0xC0000000U;
#endif
    PCC->PCCn[PCC_ADC0_INDEX]   = CLOCK_SOURCE_ADC0;
    PCC->PCCn[PCC_ADC0_INDEX]   = PCC->PCCn[PCC_ADC0_INDEX] | 0xC0000000U;
    PCC->PCCn[PCC_RTC_INDEX]    = 0xC0000000U;
    PCC->PCCn[PCC_LPTMR0_INDEX] = CLOCK_SOURCE_LPTMR0;
    PCC->PCCn[PCC_LPTMR0_INDEX] = PCC->PCCn[PCC_LPTMR0_INDEX] | 0xC0000000U;
    PCC->PCCn[PCC_PORTA_INDEX]  = 0xC0000000U;
    PCC->PCCn[PCC_PORTB_INDEX]  = 0xC0000000U;
    PCC->PCCn[PCC_PORTC_INDEX]  = 0xC0000000U;
    PCC->PCCn[PCC_PORTD_INDEX]  = 0xC0000000U;
    PCC->PCCn[PCC_PORTE_INDEX]  = 0xC0000000U;
    PCC->PCCn[PCC_FlexIO_INDEX] = CLOCK_SOURCE_FLEXIO;
    PCC->PCCn[PCC_FlexIO_INDEX] = PCC->PCCn[PCC_FlexIO_INDEX] | 0xC0000000U;
#ifdef PCC_EWM_INDEX
    PCC->PCCn[PCC_EWM_INDEX] = 0xC0000000U;
#endif
    PCC->PCCn[PCC_LPI2C0_INDEX]  = CLOCK_SOURCE_LPI2C0;
    PCC->PCCn[PCC_LPI2C0_INDEX]  = PCC->PCCn[PCC_LPI2C0_INDEX] | 0xC0000000U;
    PCC->PCCn[PCC_LPUART0_INDEX] = CLOCK_SOURCE_LPUART0;
    PCC->PCCn[PCC_LPUART0_INDEX] = PCC->PCCn[PCC_LPUART0_INDEX] | 0xC0000000U;
    PCC->PCCn[PCC_LPUART1_INDEX] = CLOCK_SOURCE_LPUART1;
    PCC->PCCn[PCC_LPUART1_INDEX] = PCC->PCCn[PCC_LPUART1_INDEX] | 0xC0000000U;
#ifdef PCC_LPUART2_INDEX
    PCC->PCCn[PCC_LPUART2_INDEX] = CLOCK_SOURCE_LPUART2;
    PCC->PCCn[PCC_LPUART2_INDEX] = PCC->PCCn[PCC_LPUART2_INDEX] | 0xC0000000U;
#endif
#ifdef PCC_FTM4_INDEX
    PCC->PCCn[PCC_FTM4_INDEX] = CLOCK_SOURCE_FTM4;
    PCC->PCCn[PCC_FTM4_INDEX] = PCC->PCCn[PCC_FTM4_INDEX] | 0xC0000000U;
#endif
#ifdef PCC_FTM5_INDEX
    PCC->PCCn[PCC_FTM5_INDEX] = CLOCK_SOURCE_FTM5;
    PCC->PCCn[PCC_FTM5_INDEX] = PCC->PCCn[PCC_FTM5_INDEX] | 0xC0000000U;
#endif
    PCC->PCCn[PCC_CMP0_INDEX] = 0xC0000000U;
#ifdef PCC_CMU0_INDEX
    PCC->PCCn[PCC_CMU0_INDEX] = 0xC0000000U;
#endif
#ifdef PCC_CMU1_INDEX
    PCC->PCCn[PCC_CMU1_INDEX] = 0xC0000000U;
#endif
#ifdef PCC_ENET_INDEX
    PCC->PCCn[PCC_ENET_INDEX] = CLOCK_SOURCE_ENET;
    PCC->PCCn[PCC_ENET_INDEX] = PCC->PCCn[PCC_ENET_INDEX] | 0xC0000000U;
#endif
}

void sircStart()
{
    do
    {
        SCG->SIRCCSR = SCG->SIRCCSR | SCG_SIRCCSR_SIRCEN_MASK;
    } while (0 == (SCG->SIRCCSR & SCG_SIRCCSR_SIRCVLD_MASK));
}

void sircStop()
{
    do
    {
        SCG->SIRCCSR = 0;
    } while (0 != (SCG->SIRCCSR & SCG_SIRCCSR_SIRCEN_MASK));
}

void sircConfig()
{
    SCG->SIRCCFG = SCG_SIRCCFG_RANGE_MASK;
    SCG->SIRCDIV = SCG_SIRCDIV_SIRCDIV2(_SIRCDIV2) | SCG_SIRCDIV_SIRCDIV1(_SIRCDIV1);
}

void sircSysClk()
{
    uint32_t const cfg = SCG_RCCR_SCS(2) | SCG_RCCR_DIVSLOW(1);
    do
    {
        SCG->RCCR = cfg;
    } while (SCG->CSR != cfg);
    SysTick->LOAD = 8000 - 1;
}

bool isSircSysClk() { return (SCG->SIRCCSR & SCG_SIRCCSR_SIRCSEL_MASK) != 0; }

bool isSircValid() { return (SCG->SIRCCSR & SCG_SIRCCSR_SIRCVLD_MASK) != 0; }

void fircStart()
{
    do
    {
        SCG->FIRCCSR = SCG->FIRCCSR | (SCG_FIRCCSR_FIRCEN_MASK | SCG_FIRCCSR_FIRCERR_MASK);
    } while (0 == (SCG->FIRCCSR & SCG_FIRCCSR_FIRCVLD_MASK));
}

void fircStop()
{
    do
    {
        SCG->FIRCCSR = SCG_FIRCCSR_FIRCERR_MASK;
    } while (0 != (SCG->FIRCCSR & SCG_FIRCCSR_FIRCEN_MASK));
}

void fircConfig()
{
    SCG->FIRCCFG = 0;
    SCG->FIRCDIV = SCG_FIRCDIV_FIRCDIV2(_FIRCDIV2) | SCG_FIRCDIV_FIRCDIV1(_FIRCDIV1);
}

void fircSysClk()
{
    uint32_t const cfg = SCG_RCCR_SCS(3) | SCG_RCCR_DIVSLOW(1);
    do
    {
        SCG->RCCR = cfg;
    } while (SCG->CSR != cfg);
    SysTick->LOAD = 48000 - 1;
}

bool isFircSysClk() { return (SCG->FIRCCSR & SCG_FIRCCSR_FIRCSEL_MASK) != 0; }

bool isFircValid() { return (SCG->FIRCCSR & SCG_FIRCCSR_FIRCVLD_MASK) != 0; }

#if PLL_AVAILABLE
void spllStart()
{
    do
    {
        SCG->SPLLCSR = SCG->SPLLCSR | (SCG_SPLLCSR_SPLLEN_MASK | SCG_SPLLCSR_SPLLERR_MASK);
    } while (0 == (SCG->SPLLCSR & SCG_SPLLCSR_SPLLVLD_MASK));
}

void spllStop()
{
    SCG->SPLLCSR = SCG->SPLLCSR & ~(SCG_SPLLCSR_SPLLCM_MASK | SCG_SPLLCSR_SPLLCMRE_MASK);
    do
    {
        SCG->SPLLCSR = SCG_SPLLCSR_SPLLERR_MASK;
    } while (0 != (SCG->SPLLCSR & SCG_SPLLCSR_SPLLEN_MASK));
}

void spllConfig()
{
    SCG->SPLLCFG = PLL_CONFIG;
    SCG->SPLLDIV = SCG_SPLLDIV_SPLLDIV2(_SPLLDIV2) | SCG_SPLLDIV_SPLLDIV1(_SPLLDIV1);
}

void spllSysClk()
{
    // assumption: _SCG_RCCR_ is configured for SPLL@80MHz
    do
    {
        SCG->RCCR = _SCG_RCCR_;
    } while (SCG->CSR != _SCG_RCCR_);
    SysTick->LOAD = 80000 - 1;
}

void spllMon()
{
    SCG->SPLLCSR = SCG->SPLLCSR | (SCG_SPLLCSR_SPLLCM_MASK | SCG_SPLLCSR_SPLLCMRE_MASK);
}

bool isSpllSysClk() { return (SCG->SPLLCSR & SCG_SPLLCSR_SPLLSEL_MASK) != 0; }

bool isSpllValid() { return (SCG->SPLLCSR & SCG_SPLLCSR_SPLLVLD_MASK) != 0; }
#endif // PLL_AVAILABLE

void soscStart()
{
    do
    {
        SCG->SOSCCSR = SCG->SOSCCSR | (SCG_SOSCCSR_SOSCEN_MASK | SCG_SOSCCSR_SOSCERR_MASK);
    } while (0 == (SCG->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK));
}

void soscStop()
{
    SCG->SOSCCSR = SCG->SOSCCSR & ~(SCG_SOSCCSR_SOSCCM_MASK | SCG_SOSCCSR_SOSCCMRE_MASK);
    do
    {
        SCG->SOSCCSR = SCG_SOSCCSR_SOSCERR_MASK;
    } while (0 != (SCG->SOSCCSR & SCG_SOSCCSR_SOSCEN_MASK));
}

void soscConfig()
{
    SCG->SOSCCFG = SCG_SOSCCFG_RANGE(3) | SCG_SOSCCFG_EREFS_MASK;
    SCG->SOSCDIV = SCG_SOSCDIV_SOSCDIV2(_SOSCDIV2) | SCG_SOSCDIV_SOSCDIV1(_SOSCDIV1);
}

void soscMon()
{
    SCG->SOSCCSR = SCG->SOSCCSR | (SCG_SOSCCSR_SOSCCM_MASK | SCG_SOSCCSR_SOSCCMRE_MASK);
}

bool isSoscSysClk() { return (SCG->SOSCCSR & SCG_SOSCCSR_SOSCSEL_MASK) != 0; }

bool isSoscValid() { return (SCG->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK) != 0; }

void rccrConfig()
{
    do
    {
        SCG->RCCR = _SCG_RCCR_;
    } while (SCG->CSR != _SCG_RCCR_);
}

} // namespace

extern "C"
{
void configurPll()
{
    disablePeripheralClocks();

    // legacy
    RCM->SRIE = RCM->SRIE | static_cast<uint32_t>(0x8f);
    RCM->SSRS = static_cast<uint32_t>(0x1c);

    bool sircNeedsConfig = true;
    if (!isSircSysClk())
    {
        sircStop();
        sircConfig();
        sircStart();
        sircSysClk();
        sircNeedsConfig = false;
    }

    if (!isFircSysClk())
    {
        fircStop();
        fircConfig();
        fircStart();
        fircSysClk();
    }

    if (sircNeedsConfig)
    {
        sircStop();
        sircConfig();
        sircStart();
    }

#if PLL_AVAILABLE && CPU_XTAL_CLK_HZ
    spllStop();
#endif

#ifdef CPU_XTAL_CLK_HZ
    soscStop();
    soscConfig();
    soscStart();
    soscMon();
#endif

#if PLL_AVAILABLE && CPU_XTAL_CLK_HZ
    spllConfig();
    spllStart();
    spllMon();
#endif

    rccrConfig();

    SCG->CLKOUTCNFG = SCG_CLKOUTCNFG_CLKOUTSEL(CLOCKoUT_SELECT);

    // legacy
    RCM->SRIE = _RCM_SRIE_;

    enablePeripheralClocks();
}

void fircOff()
{
    if (!isFircSysClk())
    {
        fircStop();
    }
    else
    {
        while (true) {} // panic!
    }
}

void extOscOff()
{
    if (!isSoscSysClk())
    {
        soscStop();
    }
    else
    {
        while (true) {} // panic!
    }
}

void pllOff()
{
    if (!isSpllSysClk())
    {
        spllStop();
    }
    else
    {
        while (true) {} // panic!
    }
}

void systemClock2Sirc()
{
    if (isSircSysClk())
    {
        return; // nothing to do
    }

    if (!isSircValid())
    {
        sircStop();
        sircConfig();
        sircStart();
    }

    sircSysClk();
}

void systemClock2Firc()
{
    if (isFircSysClk())
    {
        return; // nothing to do
    }

    if (!isFircValid())
    {
        fircStop();
        fircConfig();
        fircStart();
    }

    fircSysClk();
}

void systemClock2Pll()
{
#if PLL_AVAILABLE
    if (isSpllSysClk())
    {
        return; // nothing to do
    }

    if (!isSpllValid())
    {
        spllStop();
        if (!isSoscValid())
        {
            soscStop();
            soscConfig();
            soscStart();
            soscMon();
        }
        spllConfig();
        spllStart();
        spllMon();
    }

    spllSysClk();
#else
    while (true) {} // panic!
#endif // PLL_AVAILABLE
}
} // extern "C"
