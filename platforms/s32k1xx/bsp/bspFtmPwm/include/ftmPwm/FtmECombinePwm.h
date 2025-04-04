// Copyright 2024 Accenture.

#pragma once

#include "ftmPwm/FtmCombinePwm.h"

namespace bios
{
template<uint8_t hwChannel, uint16_t res = 10, uint8_t m = 1, uint8_t div = 1>
class FtmECombinePwm
{
public:
    FtmECombinePwm(tFtm& ftm, tFtmCombinePwmConfiguration const& config)
    : _ftm(ftm), _ch(ftm, config)
    {}

    /**
     * init after startUp
     *      prepare registers but not start
     */
    bsp::BspReturnCode init(tFtmCombinePwmConfiguration const* const config = nullptr)
    {
        _ch.init(config);
        return bsp::BSP_OK;
    }

    /**
     * start include DMA or Isr in case of configuration
     *     .. clean flags before
     */
    inline bsp::BspReturnCode start()
    {
        _ch.start();
        return bsp::BSP_OK;
    }

    /**
     * stop capture, clean flags
     */
    inline bsp::BspReturnCode stop()
    {
        _ch.stop();
        return bsp::BSP_OK;
    }

    inline uint16_t getCurrentTime() { return _ch.CurrentCounter(); }

    inline uint32_t getlPeriod() { return _ch.getlPeriod(); }

    inline bool setlPeriod(uint32_t time)
    {
        time               = time * m / div;
        uint32_t const per = _ftm.getCNTIN() + time;
        if (per < 0xFFFEU)
        {
            _ftm.setMOD(static_cast<uint16_t>(per));
            _ftm.trgSwTrigger();
            return true;
        }
        return false;
    }

    inline uint32_t getDuty() { return _ch.getDuty(); }

    inline void setDuty(uint16_t duty)
    {
        uint16_t const range = res * 100;
        if (duty > range)
        {
            duty = range;
        }
        if (false == _ch.isELSA())
        {
            duty = range - duty;
        }
        uint16_t const mod  = _ftm.getMOD();
        uint16_t const cint = _ftm.getCNTIN();
        if (duty == 0)
        {
            _ch.setC1(mod + 1);
            _ch.setC0(cint);
        }
        else if (duty == range)
        {
            _ch.setC0(mod);
            _ch.setC1(mod);
        }
        else
        {
            uint16_t const per   = mod - cint + 1;
            uint16_t const lduty = static_cast<uint16_t>(static_cast<uint32_t>(per) * duty / range);
            if (lduty < 0xFFFFU)
            {
                _ch.setC1(mod);
                _ch.setC0(cint + lduty);
            }
        }
        _ftm.trgSwTrigger();
    }

    inline uint16_t validDuty(uint16_t const duty) { return _ch.validDuty(duty); }

    inline uint8_t getChannel() { return hwChannel; }

    inline void clrEvent() { _ch.clrEvent(); }

    /**
     * read actual capture pin state
     */
    inline bool getPinState() { return _ch.getPinState(); }

private:
    tFtm& _ftm;
    FtmCombinePwm<hwChannel, res, m, div> _ch;
};

} // namespace bios
