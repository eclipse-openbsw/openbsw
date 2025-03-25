// Copyright 2024 Accenture.

#pragma once

#include "bsp/Bsp.h"
#include "ftm/Ftm.h"
#include "io/Io.h"

namespace bios
{
struct tFtmCPwmConfiguration
{
    uint8_t ctrl;
    bool interruptActive;
    bool dmaActive;
    bool icrst;
    Io::PinId pin;
    uint16_t minDuty;
    uint16_t maxDuty;
};

template<uint8_t hwChannel, uint16_t res = 10, uint8_t m = 1, uint8_t div = 1>
class FtmCPwm
{
public:
    FtmCPwm(tFtm& ftm, tFtmCPwmConfiguration const& config)
    : _ftm(ftm), _HwChannel(_ftm.getChannel(hwChannel)), _configuration(&config)
    {}

    /**
     * init after startUp
     *      prepare registers but not start
     */
    bsp::BspReturnCode init(tFtmCPwmConfiguration const* const config = nullptr)
    {
        if (config != nullptr)
        {
            _configuration = config;
        }
        _ftm.writeProtectionDisable(true);
        _HwChannel.sc = 0;

        _HwChannel.sc |= (_configuration->ctrl & tFtm::CTRL_ELSx_MASK) << 2;
        _HwChannel.sc |= (_configuration->ctrl & tFtm::CTRL_MSx_MASK) << 2;

        clrEvent();

        // combine link settings
        _ftm.setCombine(
            hwChannel,
            (_configuration->ctrl & tFtm::CTRL_COMBINE_MASK) != 0,
            (_configuration->ctrl & tFtm::CTRL_MCOMBINE_MASK) != 0,
            (_configuration->ctrl & tFtm::CTRL_DECAPEN_MASK) != 0);
        _ftm.setCPWMS((_configuration->ctrl & tFtm::CTRL_CPWMS_MASK) != 0);

        (void)Io::setDefaultConfiguration(_configuration->pin);

        return bsp::BSP_OK;
    }

    /**
     * start include DMA or Isr in case of configuration
     *     .. clean flags before
     */
    inline bsp::BspReturnCode start()
    {
        _ftm.writeProtectionDisable(true);
        clrEvent();
        _ftm.setPWMen(hwChannel, true);
        if (_configuration->interruptActive)
        {
            _HwChannel.sc |= FTM_CnSC_CHIE_MASK;
        }
        else
        {
            _HwChannel.sc &= ~FTM_CnSC_CHIE_MASK;
        }
        if (_configuration->dmaActive)
        {
            _HwChannel.sc |= FTM_CnSC_DMA_MASK;
        }
        else
        {
            _HwChannel.sc &= ~FTM_CnSC_DMA_MASK;
        }
        setDuty(0);
        return bsp::BSP_OK;
    }

    /**
     * stop capture, clean flags
     */
    inline bsp::BspReturnCode stop()
    {
        _ftm.setPWMen(hwChannel, false);
        setDuty(0);
        _ftm.writeProtectionDisable(true);
        _HwChannel.sc &= ~FTM_CnSC_CHIE_MASK;
        _HwChannel.sc &= ~FTM_CnSC_DMA_MASK;
        clrEvent();

        return bsp::BSP_OK;
    }

    inline uint16_t getCurrentTime() { return _ftm.CurrentCounter(); }

    inline uint32_t getlPeriod()
    {
        return static_cast<uint32_t>(_ftm.getMOD() - _ftm.getCNTIN()) * 2 * m / div;
    }

    inline bool setlPeriod() { return false; }

    inline uint32_t getDuty()
    {
        uint16_t const range = res * 100;
        uint32_t const per   = static_cast<uint32_t>(_ftm.getMOD() - _ftm.getCNTIN());
        uint32_t duty        = _HwChannel.v;
        if (duty > per)
        {
            duty = range;
        }
        else
        {
            duty = duty * range / per;
        }
        if ((_HwChannel.sc & FTM_CnSC_ELSA_MASK) != 0)
        {
            return range - duty;
        }
        else
        {
            return duty;
        }
    }

    inline void setDuty(uint16_t duty)
    {
        uint16_t const range = res * 100;
        if (duty > range)
        {
            duty = range;
        }
        if ((_HwChannel.sc & FTM_CnSC_ELSA_MASK) != 0)
        {
            duty = range - duty;
        }
        if (duty == 0)
        {
            _HwChannel.v = 0;
        }
        else if (duty == range)
        {
            _HwChannel.v = 0x7FFFU;
        }
        else
        {
            uint16_t const per   = _ftm.getMOD() - _ftm.getCNTIN();
            uint16_t const lduty = static_cast<uint16_t>(static_cast<uint32_t>(per) * duty / range);
            if (lduty < 0x7FFEU)
            {
                _HwChannel.v = lduty;
            }
        }
    }

    inline uint16_t validDuty(uint16_t const duty)
    {
        if (duty < _configuration->minDuty)
        {
            return 0;
        }
        if (duty > _configuration->maxDuty)
        {
            return 100 * res;
        }
        return duty;
    }

    inline uint8_t getChannel() { return hwChannel; }

    inline void clrEvent() { _HwChannel.sc &= ~FTM_CnSC_CHF_MASK; }

    inline bool getEvent() { return (_HwChannel.sc & FTM_CnSC_CHF_MASK) != 0; }

    /**
     * read actual capture pin state
     */
    inline bool getPinState() { return (_HwChannel.sc & FTM_CnSC_CHIS_MASK) != 0; }

    inline bool getOverflowEvent() { return _ftm.getOverflowEvent(); }

    inline void clrOverflowEvent() { _ftm.clrOverflowEvent(); }

    inline bool getReloadEvent() { return _ftm.getReloadEvent(); }

    inline void clrReloadEvent() { _ftm.clrReloadEvent(); }

private:
    tFtm& _ftm;
    tFtmChannelConfiguration& _HwChannel;
    tFtmCPwmConfiguration const* _configuration;
};

} // namespace bios

