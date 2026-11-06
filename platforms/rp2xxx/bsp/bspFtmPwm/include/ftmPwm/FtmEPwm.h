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
        // return static_cast<uint32_t>(_ftm.getMOD() - _ftm.getCNTIN() + 1) * m / div;
        return {};
    }

    inline bool setlPeriod(uint32_t time) { return false; }

    inline uint32_t getDuty() { return {}; }

    inline void setDuty(uint16_t duty) {}

    inline uint16_t validDuty(uint16_t const duty) { return duty; }

    inline uint8_t getChannel()
    {
        // return hwChannel;
        return {};
    }

    inline void clrEvent()
    {
        //_HwChannel.sc = _HwChannel.sc & ~FTM_CnSC_CHF_MASK;
    }

    inline bool getEvent()
    {
        // return (_HwChannel.sc & FTM_CnSC_CHF_MASK) != 0;
        return {};
    }

    /**
     * read actual capture pin state
     */
    inline bool getPinState()
    {
        // return (_HwChannel.sc & FTM_CnSC_CHIS_MASK) != 0;
        return {};
    }

    inline bool getOverflowEvent()
    {
        // return _ftm.getOverflowEvent();
        return {};
    }

    inline void clrOverflowEvent()
    {
        //_ftm.clrOverflowEvent();
    }

    inline bool getReloadEvent()
    {
        // return _ftm.getReloadEvent();
        return {};
    }

    inline void clrReloadEvent()
    {
        //_ftm.clrReloadEvent();
    }

private:
    tFtm& _ftm;
    tFtmChannelConfiguration& _HwChannel;
    tFtmEPwmConfiguration const* _configuration;
};

} // namespace bios
