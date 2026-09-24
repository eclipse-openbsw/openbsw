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
        // return _ch.CurrentCounter();
        return {};
    }

    inline uint32_t getlPeriod()
    {
        // return _ch.getlPeriod();
        return {};
    }

    inline bool setlPeriod() { return false; }

    inline uint32_t getDuty()
    {
        // return _ch.getDuty();
        return {};
    }

    inline void setDuty(uint16_t duty) {}

    inline void setDutyInv() {}

    inline uint16_t validDuty(uint16_t const duty)
    {
        // return _ch.validDuty(duty);
        return {};
    }

    inline uint8_t getChannel() { return hwChannel; }

    inline void clrEvent()
    {
        // _ch.clrEvent();
    }

    /**
     * read actual capture pin state
     */
    inline bool getPinState()
    {
        // return _ch.getPinState();
        return {};
    }

private:
    tFtm& _ftm;
    FtmCombinePwm<hwChannel, res, m, div> _ch;
};

} // namespace bios
