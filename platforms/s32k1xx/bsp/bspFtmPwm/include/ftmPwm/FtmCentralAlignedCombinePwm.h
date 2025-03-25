// Copyright 2024 Accenture.

#pragma once

#include "ftmPwm/FtmCombinePwm.h"

namespace bios
{
template<uint8_t hwChannel, uint16_t res = 10, uint8_t m = 1, uint8_t div = 1>
class FtmCentAligCombinePwm
{
public:
    FtmCentAligCombinePwm(tFtm& ftm, tFtmCombinePwmConfiguration const& config)
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

    inline bool setlPeriod() { return false; }

    inline uint32_t getDuty() { return _ch.getDuty(); }

    inline void setDuty(uint16_t duty)
    {
        uint16_t const range = res * 100;
        if (duty > range)
        {
            duty = range;
        }
        uint16_t const mod  = _ftm.getMOD();
        uint16_t const cint = _ftm.getCNTIN();
        if (duty == 0)
        {
            _ch.setC0(mod);
            _ch.setC1(mod);
        }
        else if (duty == range)
        {
            _ch.setC1(mod + 1);
            _ch.setC0(cint);
        }
        else
        {
            uint16_t const per   = mod - cint + 1;
            uint16_t const lduty = static_cast<uint16_t>(static_cast<uint32_t>(per) * duty / range);
            if (lduty < 0xFFFFU)
            {
                _ch.setC0(cint + (mod - cint) / 2 - lduty / 2);
                _ch.setC1(cint + (mod - cint) / 2 + lduty / 2);
            }
        }
        _ftm.trgSwTrigger();
    }

    inline void setDutyInv()
    {
        uint16_t const mod       = _ftm.getMOD();
        uint16_t const cint      = _ftm.getCNTIN();
        uint16_t const firstEdge = _ch.getC0();
        uint16_t secEdge         = firstEdge - cint;
        secEdge                  = mod - secEdge;
        _ch.setC1(secEdge);
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

