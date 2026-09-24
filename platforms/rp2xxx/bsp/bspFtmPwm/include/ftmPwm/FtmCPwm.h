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
        // return static_cast<uint32_t>(_ftm.getMOD() - _ftm.getCNTIN()) * 2 * m / div;
        return {};
    }

    inline bool setlPeriod() { return false; }

    inline uint32_t getDuty() { return {}; }

    inline void setDuty(uint16_t duty) {}

    inline uint16_t validDuty(uint16_t const duty) { return duty; }

    inline uint8_t getChannel() { return {}; }

    inline void clrEvent() {}

    inline bool getEvent() { return {}; }

    /**
     * read actual capture pin state
     */
    inline bool getPinState() { return {}; }

    inline bool getOverflowEvent() { return {}; }

    inline void clrOverflowEvent() {}

    inline bool getReloadEvent() { return {}; }

    inline void clrReloadEvent() {}

private:
    tFtm& _ftm;
    tFtmChannelConfiguration& _HwChannel;
    tFtmCPwmConfiguration const* _configuration;
};

} // namespace bios
