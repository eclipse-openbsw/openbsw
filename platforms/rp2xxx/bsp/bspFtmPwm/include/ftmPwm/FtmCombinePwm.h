/********************************************************************************
 * Copyright (c) 2024 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "bsp/Bsp.h"
#include "ftm/Ftm.h"
#include "io/Io.h"

namespace bios
{
struct tFtmCombinePwmConfiguration
{
    uint8_t ctrl0;
    uint8_t ctrl1;
    bool interruptActive0;
    bool interruptActive1;
    bool dmaActive0;
    bool dmaActive1;
    Io::PinId pin;
    uint16_t minDuty;
    uint16_t maxDuty;
};

template<uint8_t hwChannel, uint16_t res = 10, uint8_t m = 1, uint8_t div = 1>
class FtmCombinePwm
{
public:
    FtmCombinePwm(tFtm& ftm, tFtmCombinePwmConfiguration const& config)
    : _ftm(ftm)
    , _HwChannel0(_ftm.getChannel((hwChannel / 2) * 2))
    , _HwChannel1(_ftm.getChannel((hwChannel / 2) * 2 + 1))
    , _configuration(&config)
    {}

    /**
     * init after startUp
     *      prepare registers but not start
     */
    bsp::BspReturnCode init(tFtmCombinePwmConfiguration const* const config = nullptr)
    {
        return bsp::BSP_OK;
    }

    /**
     * start include DMA or Isr in case of configuration
     *     .. clean flags before
     */
    inline bsp::BspReturnCode start() { return bsp::BSP_OK; }

    /**
     * stop capture, clean flags
     */
    inline bsp::BspReturnCode stop() { return bsp::BSP_OK; }

    inline uint16_t getCurrentTime()
    {
        // return _ftm.CurrentCounter();
        return {};
    }

    inline uint32_t getlPeriod()
    {
        //        return static_cast<uint32_t>(_ftm.getMOD() - _ftm.getCNTIN() + 1) * m / div;
        return {};
    }

    inline bool setlPeriod() { return false; }

    inline uint32_t getDuty() { return {}; }

    inline void setDuty(uint16_t duty) {}

    inline uint16_t validDuty(uint16_t const duty) { return {}; }

    inline void setC0(uint16_t const d)
    {
        // _HwChannel0.v = d;
    }

    inline void setC1(uint16_t const d)
    {
        //_HwChannel1.v = d;
    }

    inline uint16_t getC0()
    {
        //        return static_cast<uint16_t>(_HwChannel0.v);
        return {};
    }

    inline uint16_t getC1()
    {
        //        return static_cast<uint16_t>(_HwChannel1.v);
        return {};
    }

    inline void setC0Relativ(uint16_t d) {}

    inline void setC1Relativ(uint16_t d) {}

    inline void forceReload()
    {
        //(void)_ftm.forceReload();
    }

    inline uint8_t getChannel()
    {
        // return hwChannel;
        return {};
    }

    inline void clrEvent() {}

    inline bool getEvent0()
    {
        // return (_HwChannel0.sc & FTM_CnSC_CHF_MASK) != 0;
        return {};
    }

    inline bool getEvent1()
    {
        //        return (_HwChannel1.sc & FTM_CnSC_CHF_MASK) != 0;
        return {};
    }

    /**
     * read actual capture pin state
     */
    inline bool getPinState() { return {}; }

    inline bool getOverflowEvent() { return {}; }

    inline void clrOverflowEvent() {}

    inline bool getReloadEvent() { return {}; }

    inline void clrReloadEvent() {}

    inline bool isELSA() { return {}; }

private:
    tFtm& _ftm;
    tFtmChannelConfiguration& _HwChannel0;
    tFtmChannelConfiguration& _HwChannel1;
    tFtmCombinePwmConfiguration const* _configuration;
};

} // namespace bios
