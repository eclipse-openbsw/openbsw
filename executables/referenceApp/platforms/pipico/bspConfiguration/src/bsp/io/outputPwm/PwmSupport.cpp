/********************************************************************************
 * Copyright (c) 2024 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "bsp/io/outputPwm/PwmSupport.h"

#include "bsp/io/outputPwm/PwmSupportConfiguration.hpp"
#include "outputPwm/OutputPwm.h"

namespace bios
{
PwmSupport::PwmSupport(/*tFtm& ftm4*/) {}

void PwmSupport::init() {}

void PwmSupport::start() {}

void PwmSupport::stop() {}

void PwmSupport::shutdown() { stop(); }

bsp::BspReturnCode PwmSupport::setDuty(uint16_t chan, uint16_t duty, bool /* immediateUpdate */)
{
    switch (chan)
    {
        default: return bsp::BSP_NOT_SUPPORTED;
    }
    return bsp::BSP_OK;
}

/*
 * Set period for specific channel. Unit: microseconds
 */
bsp::BspReturnCode PwmSupport::setPeriod(uint16_t chan, uint16_t period)
{
    switch (chan)
    {
        default: return bsp::BSP_NOT_SUPPORTED;
    }
    return bsp::BSP_OK;
}

} // namespace bios
