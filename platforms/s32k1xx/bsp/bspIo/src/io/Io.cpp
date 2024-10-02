// Copyright 2024 Accenture.

#include "io/Io.h"

#include "mcu/mcu.h"

using namespace bsp;

namespace bios
{

#undef BSP_IO_PIN_CONFIGURATION
#define BSP_IO_PIN_CONFIGURATION 1
#include "bsp/io/io/ioConfiguration.h"

GPIO_Type* const Io::gpioPtrs[_PORTS_MAX_]       = GPIO_BASE_PTRS;
PORT_Type* const Io::gpioPrfCfgPtrs[_PORTS_MAX_] = PORT_BASE_PTRS;

BspReturnCode Io::setDefaultConfiguration(uint16_t io)
{
    if (io < NUMBER_OF_IOS)
    {
        return setConfiguration(io, fPinConfiguration[io]);
    }
    else
    {
        return BSP_NOT_SUPPORTED;
    }
}

BspReturnCode Io::setConfiguration(uint16_t io, PinConfiguration const& cfg)
{
    if (io < NUMBER_OF_IOS)
    {
        if (cfg.pinNumber != fPinConfiguration[io].pinNumber
            || cfg.port != fPinConfiguration[io].port)
        {
            // Mapping of IOs must be the same as default
            return BSP_ERROR;
        }
        if (cfg.pinNumber < MAX_GPIO_NUMBER)
        {
            GPIO_Type* portBase = (GPIO_Type*)(gpioPtrs[cfg.port]);
            PORT_Type* cfgBase  = (PORT_Type*)(gpioPrfCfgPtrs[cfg.port]);
            // disable
            portBase->PIDR |= (1 << cfg.pinNumber); // default High-Z
            if (_IN == cfg.dir)
            {
                // filter settings ..
                cfgBase->DFER &= ~(1 << cfg.pinNumber);
                if (cfg.pinCfgExt & FILTER_ACTIVE)
                {
                    if (cfg.pinCfgExt & FILTER_CLOCK_LPO)
                    {
                        cfgBase->DFCR = 1;
                    }
                    else
                    {
                        cfgBase->DFCR = 0;
                    }
                    cfgBase->DFWR = (cfg.pinCfgExt & 0x1f);
                    cfgBase->DFER |= (1 << cfg.pinNumber);
                }
                portBase->PDDR &= ~(1 << cfg.pinNumber);
                portBase->PIDR &= ~(1 << cfg.pinNumber);
            }
            else if (_OUT == cfg.dir)
            {
                portBase->PDDR |= (1 << cfg.pinNumber);
                portBase->PIDR &= ~(1 << cfg.pinNumber);
            }
            else
            {
                // still disable
            }
            cfgBase->PCR[cfg.pinNumber] = cfg.pinCfg;
            if (cfg.pinCfg & clearISR)
            {
                cfgBase->PCR[cfg.pinNumber] |= clearISR;
            }
        }
        return BSP_OK;
    }
    else
    {
        return BSP_NOT_SUPPORTED;
    }
}

bsp::BspReturnCode Io::getConfiguration(uint16_t io, PinConfiguration& cfg)
{
    if (io >= NUMBER_OF_IOS)
    {
        return BSP_NOT_SUPPORTED;
    }
    cfg.port                  = fPinConfiguration[io].port;
    cfg.pinNumber             = fPinConfiguration[io].pinNumber;
    GPIO_Type* const portBase = gpioPtrs[cfg.port];
    PORT_Type* const cfgBase  = gpioPrfCfgPtrs[cfg.port];
    cfg.pinCfg                = cfgBase->PCR[cfg.pinNumber];
    cfg.pinCfgExt             = 0;
    if (portBase->PIDR & (1U << cfg.pinNumber))
    {
        cfg.dir = _DISABLED;
    }
    else if (portBase->PDDR & (1U << cfg.pinNumber))
    {
        cfg.dir = _OUT;
    }
    else
    {
        cfg.dir = _IN;
        if (cfgBase->DFER & (1U << cfg.pinNumber))
        {
            cfg.pinCfgExt = FILTER_ACTIVE;
            cfg.pinCfgExt |= (cfgBase->DFCR) ? FILTER_CLOCK_LPO : 0;
            cfg.pinCfgExt |= cfgBase->DFWR & 0x1FU;
        }
    }
    return BSP_OK;
}

uint32_t Io::getPinNumber(uint16_t io) { return fPinConfiguration[io].pinNumber; }

/**
 * @disc: get pin
 * @par : pin from configuration enum
 * @ret : true - 1, false 0
 */
bool Io::getPin(uint16_t io)
{
    if ((io < NUMBER_OF_IOS) && (fPinConfiguration[io].pinNumber < MAX_GPIO_NUMBER))
    {
        return (
            (gpioPtrs[fPinConfiguration[io].port]->PDIR & (1 << fPinConfiguration[io].pinNumber))
            != 0);
    }
    else
    {
        return false;
    }
}

bool Io::getPinIrq(uint16_t io)
{
    if ((io < NUMBER_OF_IOS) && (fPinConfiguration[io].pinNumber < MAX_GPIO_NUMBER))
    {
        PORT_Type* cfgBase = (PORT_Type*)(gpioPrfCfgPtrs[fPinConfiguration[io].port]);
        return ((cfgBase->PCR[fPinConfiguration[io].pinNumber] & clearISR) != 0);
    }
    else
    {
        return false;
    }
}

BspReturnCode Io::setPin(uint16_t io, bool level)
{
    if ((io < NUMBER_OF_IOS) && (fPinConfiguration[io].pinNumber < MAX_GPIO_NUMBER))
    {
        GPIO_Type* portBase = (GPIO_Type*)(gpioPtrs[fPinConfiguration[io].port]);
        if (level)
        {
            portBase->PSOR = (1 << fPinConfiguration[io].pinNumber);
        }
        else
        {
            portBase->PCOR = (1 << fPinConfiguration[io].pinNumber);
        }
        return BSP_OK;
    }
    else
    {
        return BSP_NOT_SUPPORTED;
    }
}

bsp::BspReturnCode Io::resetConfig(uint16_t io)
{
    if ((io < NUMBER_OF_IOS) && (fPinConfiguration[io].pinNumber < MAX_GPIO_NUMBER))
    {
        GPIO_Type* portBase = (GPIO_Type*)(gpioPtrs[fPinConfiguration[io].port]);
        PORT_Type* cfgBase  = (PORT_Type*)(gpioPrfCfgPtrs[fPinConfiguration[io].port]);
        // disable
        portBase->PIDR |= (1 << fPinConfiguration[io].pinNumber); // default High-Z
        portBase->PDDR &= ~(1 << fPinConfiguration[io].pinNumber);
        cfgBase->DFER &= ~(1 << fPinConfiguration[io].pinNumber);
        cfgBase->PCR[fPinConfiguration[io].pinNumber] = 0;
        return BSP_OK;
    }
    else
    {
        return BSP_NOT_SUPPORTED;
    }
}

} /* namespace bios */
