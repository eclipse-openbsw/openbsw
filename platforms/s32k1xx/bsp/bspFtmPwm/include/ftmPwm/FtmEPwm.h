// Copyright 2024 Accenture.

#pragma once

#include "bsp/Bsp.h"
#include "ftm/Ftm.h"
#include "io/Io.h"

namespace bios
{
struct tFtmEPwmConfiguration
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
class FtmEPwm
{
public:
    FtmEPwm(tFtm& ftm, tFtmEPwmConfiguration const& config)
    : _ftm(ftm), _HwChannel(_ftm.getChannel(hwChannel)), _configuration(&config)
    {}

    /**
     * init after startUp
     *      prepare registers but not start
     */
    bsp::BspReturnCode init(tFtmEPwmConfiguration const* const config = nullptr)
    {
        if (config != nullptr)
        {
            _configuration = config;
        }
        _ftm.writeProtectionDisable(true);
        _HwChannel.sc = 0;

        _HwChannel.sc = _HwChannel.sc | ((_configuration->ctrl & tFtm::CTRL_ELSx_MASK) << 2);
        _HwChannel.sc = _HwChannel.sc | ((_configuration->ctrl & tFtm::CTRL_MSx_MASK) << 2);

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
            _HwChannel.sc = _HwChannel.sc | FTM_CnSC_CHIE_MASK;
        }
        else
        {
            _HwChannel.sc = _HwChannel.sc & ~FTM_CnSC_CHIE_MASK;
        }
        if (_configuration->dmaActive)
        {
            _HwChannel.sc = _HwChannel.sc | FTM_CnSC_DMA_MASK;
        }
        else
        {
            _HwChannel.sc = _HwChannel.sc & ~FTM_CnSC_DMA_MASK;
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
        _HwChannel.sc = _HwChannel.sc & ~FTM_CnSC_CHIE_MASK;
        _HwChannel.sc = _HwChannel.sc & ~FTM_CnSC_DMA_MASK;
        clrEvent();

        return bsp::BSP_OK;
    }

    inline uint16_t getCurrentTime() { return _ftm.CurrentCounter(); }

    inline uint32_t getlPeriod()
    {
        return static_cast<uint32_t>(_ftm.getMOD() - _ftm.getCNTIN() + 1) * m / div;
    }

    inline bool setlPeriod(uint32_t time)
    {
        time               = time * m / div;
        uint32_t const per = _ftm.getCNTIN() + time;
        if (per < 0xFFFEU)
        {
            _ftm.setMOD(static_cast<uint16_t>(per));
            return true;
        }
        return false;
    }

    inline uint32_t getDuty()
    {
        uint16_t const range = res * 100;
        uint32_t const per   = static_cast<uint32_t>(_ftm.getMOD() - _ftm.getCNTIN() + 1);
        uint32_t const duty  = _HwChannel.v - _ftm.getCNTIN();
        if (duty > per)
        {
            return range;
        }
        else
        {
            return duty * range / per;
        }
    }

    inline void setDuty(uint16_t duty)
    {
        uint16_t const range = res * 100;
        if (duty > range)
        {
            duty = range;
        }
        if (duty == 0)
        {
            _HwChannel.v = _ftm.getCNTIN();
        }
        else if (duty == range)
        {
            _HwChannel.v = static_cast<uint32_t>(_ftm.getMOD() + 1);
        }
        else
        {
            uint16_t const per   = _ftm.getMOD() - _ftm.getCNTIN() + 1;
            uint16_t const lduty = static_cast<uint16_t>(
                static_cast<uint32_t>(per) * duty / range + _ftm.getCNTIN());
            if (lduty < 0xFFFFU)
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

    inline void clrEvent() { _HwChannel.sc = _HwChannel.sc & ~FTM_CnSC_CHF_MASK; }

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
    tFtmEPwmConfiguration const* _configuration;
};

} // namespace bios

