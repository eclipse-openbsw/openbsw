// Copyright 2024 Accenture.

#pragma once

#include "bsp/Bsp.h"
#include "ftm/Ftm.h"
#include "io/Io.h"

namespace bios
{
struct tFtmModCombinePwmConfiguration
{
    uint8_t ctrl0;
    uint8_t ctrl1;
    bool interruptActive0;
    bool interruptActive1;
    bool dmaActive0;
    bool dmaActive1;
    Io::PinId pin;
    uint16_t offsetFirstEdge;
    uint16_t minDuty;
    uint16_t maxDuty;
};

template<uint8_t hwChannel, uint16_t res = 10, uint8_t m = 1, uint8_t div = 1>
class FtmModCombinePwm
{
public:
    FtmModCombinePwm(tFtm& ftm, tFtmModCombinePwmConfiguration const& config)
    : _ftm(ftm)
    , _HwChannel0(_ftm.getChannel((hwChannel / 2) * 2))
    , _HwChannel1(_ftm.getChannel((hwChannel / 2) * 2 + 1))
    , _configuration(&config)
    {}

    /**
     * init after startUp
     *      prepare registers but not start
     */
    bsp::BspReturnCode init(tFtmModCombinePwmConfiguration const* const config = nullptr)
    {
        if (config != nullptr)
        {
            _configuration = config;
        }
        _ftm.writeProtectionDisable(true);
        _HwChannel0.sc = 0;
        _HwChannel1.sc = 0;

        _HwChannel0.sc = _HwChannel0.sc | ((_configuration->ctrl0 & tFtm::CTRL_ELSx_MASK) << 2);
        _HwChannel0.sc = _HwChannel0.sc | ((_configuration->ctrl0 & tFtm::CTRL_MSx_MASK) << 2);
        _HwChannel1.sc = _HwChannel1.sc | ((_configuration->ctrl1 & tFtm::CTRL_ELSx_MASK) << 2);
        _HwChannel1.sc = _HwChannel1.sc | ((_configuration->ctrl1 & tFtm::CTRL_MSx_MASK) << 2);
        clrEvent();

        // combine link settings

        uint8_t comb = 0;
        if ((_configuration->ctrl0 & tFtm::CTRL_COMBINE_MASK) != 0)
        {
            comb |= tFtm::COMBINE_COMB;
        }
        if ((_configuration->ctrl0 & tFtm::CTRL_MCOMBINE_MASK) != 0)
        {
            comb |= tFtm::COMBINE_MCOMBINE;
        }
        if ((_configuration->ctrl0 & tFtm::CTRL_DECAPEN_MASK) != 0)
        {
            comb |= tFtm::COMBINE_DECAPEN;
        }
        _ftm.setCombine((hwChannel / 2) * 2, comb);

        _ftm.setCPWMS((_configuration->ctrl0 & tFtm::CTRL_CPWMS_MASK) != 0);

        uint32_t const cint = _ftm.getCNTIN();
        _HwChannel0.v       = cint + _configuration->offsetFirstEdge;

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
        _ftm.setPWMen((hwChannel / 2) * 2, true);
        _ftm.setPWMen((hwChannel / 2) * 2 + 1, true);
        if (_configuration->interruptActive0)
        {
            _HwChannel0.sc = _HwChannel0.sc | FTM_CnSC_CHIE_MASK;
        }
        else
        {
            _HwChannel0.sc = _HwChannel0.sc & ~FTM_CnSC_CHIE_MASK;
        }
        if (_configuration->interruptActive1)
        {
            _HwChannel1.sc = _HwChannel1.sc | FTM_CnSC_CHIE_MASK;
        }
        else
        {
            _HwChannel1.sc = _HwChannel1.sc & ~FTM_CnSC_CHIE_MASK;
        }
        if (_configuration->dmaActive0)
        {
            _HwChannel0.sc = _HwChannel0.sc | FTM_CnSC_DMA_MASK;
        }
        else
        {
            _HwChannel0.sc = _HwChannel0.sc & ~FTM_CnSC_DMA_MASK;
        }
        if (_configuration->dmaActive1)
        {
            _HwChannel1.sc = _HwChannel1.sc | FTM_CnSC_DMA_MASK;
        }
        else
        {
            _HwChannel1.sc = _HwChannel1.sc & ~FTM_CnSC_DMA_MASK;
        }
        setDuty(0);
        return bsp::BSP_OK;
    }

    /**
     * stop capture, clean flags
     */
    inline bsp::BspReturnCode stop()
    {
        _ftm.writeProtectionDisable(true);
        _ftm.setPWMen((hwChannel / 2) * 2, false);
        _ftm.setPWMen((hwChannel / 2) * 2 + 1, false);
        setDuty(0);
        _HwChannel0.sc = _HwChannel0.sc & ~FTM_CnSC_CHIE_MASK;
        _HwChannel0.sc = _HwChannel0.sc & ~FTM_CnSC_DMA_MASK;
        _HwChannel1.sc = _HwChannel1.sc & ~FTM_CnSC_CHIE_MASK;
        _HwChannel1.sc = _HwChannel1.sc & ~FTM_CnSC_DMA_MASK;
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
            _ftm.trgSwTrigger();
            return true;
        }
        return false;
    }

    inline uint32_t getDuty()
    {
        uint32_t const per = static_cast<uint32_t>(_ftm.getMOD() - _ftm.getCNTIN() + 1);
        uint16_t const c0  = static_cast<uint16_t>(_HwChannel0.v);
        uint16_t const c1  = static_cast<uint16_t>(_HwChannel1.v);
        uint32_t duty      = 0;
        if (c1 >= c0)
        {
            duty = static_cast<uint32_t>(c1 - c0);
        }
        if (c0 < _ftm.getCNTIN())
        {
            duty = static_cast<uint32_t>(c0 - c1);
        }
        if (duty >= per)
        {
            return static_cast<uint32_t>(res * 100);
        }
        else
        {
            return duty * 100 * res / per;
        }
    }

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
            _HwChannel1.v = static_cast<uint32_t>(cint + _configuration->offsetFirstEdge);
        }
        else if (duty == range)
        {
            _HwChannel1.v = static_cast<uint32_t>(mod + 1);
        }
        else
        {
            uint16_t const per   = mod - cint + 1;
            uint16_t const lduty = static_cast<uint16_t>(static_cast<uint32_t>(per) * duty / range);
            if (lduty < 0xFFFFU)
            {
                uint32_t c1 = static_cast<uint32_t>(cint) + _configuration->offsetFirstEdge + lduty;
                if (c1 > mod)
                {
                    c1 = c1 - mod;
                    c1 = c1 + cint + 1;
                }
                _HwChannel1.v = static_cast<uint16_t>(c1);
            }
        }
    }

    inline uint16_t validDuty(uint16_t const duty)
    {
        if (duty < _configuration->minDuty)
        {
            return 0;
        }
        else if (duty > _configuration->maxDuty)
        {
            return 100 * res;
        }
        else
        {
            return duty;
        }
    }

    inline void setC0(uint16_t const d) { _HwChannel0.v = d; }

    inline void setC1(uint16_t const d) { _HwChannel1.v = d; }

    inline void setC1Relativ(uint16_t const d)
    {
        uint16_t const cint = _ftm.getCNTIN();
        uint16_t const mod  = _ftm.getMOD();
        uint32_t const _d   = static_cast<uint32_t>(d) + cint;
        if (_d < mod)
        {
            _HwChannel1.v = d;
        }
        else
        {
            _HwChannel1.v = mod;
        }
    }

    inline uint8_t getChannel() { return hwChannel; }

    inline void clrEvent()
    {
        _HwChannel0.sc = _HwChannel0.sc & ~FTM_CnSC_CHF_MASK;
        _HwChannel1.sc = _HwChannel1.sc & ~FTM_CnSC_CHF_MASK;
    }

    inline bool getEvent0() { return (_HwChannel0.sc & FTM_CnSC_CHF_MASK) != 0; }

    inline bool getEvent1() { return (_HwChannel1.sc & FTM_CnSC_CHF_MASK) != 0; }

    /**
     * read actual capture pin state
     */
    inline bool getPinState() { return (_HwChannel0.sc & FTM_CnSC_CHIS_MASK) != 0; }

    inline bool getOverflowEvent() { return _ftm.getOverflowEvent(); }

    inline void clrOverflowEvent() { _ftm.clrOverflowEvent(); }

    inline bool getReloadEvent() { return _ftm.getReloadEvent(); }

    inline void clrReloadEvent() { _ftm.clrReloadEvent(); }

    inline bool isELSA() { return (_HwChannel0.sc & FTM_CnSC_ELSA_MASK) != 0; }

private:
    tFtm& _ftm;
    tFtmChannelConfiguration& _HwChannel0;
    tFtmChannelConfiguration& _HwChannel1;
    tFtmModCombinePwmConfiguration const* _configuration;
};

} // namespace bios

