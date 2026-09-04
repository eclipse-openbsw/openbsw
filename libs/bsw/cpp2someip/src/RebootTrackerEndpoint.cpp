/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/RebootTrackerEndpoint.h"

namespace someip
{
RebootTrackerEndpoint::RebootTrackerEndpoint()
: _latestUnicastSessionId(0U)
, _latestMulticastSessionId(0U)
, _latestUnicastRebootFlag(false)
, _latestMulticastRebootFlag(false)
{}

void RebootTrackerEndpoint::setUnicast(uint16_t const sessionId, bool const rebootFlag)
{
    _latestUnicastSessionId  = sessionId;
    _latestUnicastRebootFlag = rebootFlag;
}

void RebootTrackerEndpoint::setMulticast(uint16_t const sessionId, bool const rebootFlag)
{
    _latestMulticastSessionId  = sessionId;
    _latestMulticastRebootFlag = rebootFlag;
}

bool RebootTrackerEndpoint::getMulticastRebootValue(
    uint16_t const sessionId, bool const rebootFlag) const
{
    if (rebootFlag == false)
    {
        return false;
    }
    if (_latestMulticastRebootFlag == false)
    {
        return true;
    }

    return _latestMulticastSessionId >= sessionId;
}

bool RebootTrackerEndpoint::getUnicastRebootValue(
    uint16_t const sessionId, bool const rebootFlag) const
{
    if (rebootFlag == false)
    {
        return false;
    }
    if (_latestUnicastRebootFlag == false)
    {
        return true;
    }

    return _latestUnicastSessionId >= sessionId;
}

void RebootTrackerEndpoint::initActiveReboot()
{
    _latestUnicastSessionId    = 0U;
    _latestMulticastSessionId  = 0U;
    _latestUnicastRebootFlag   = true; // true to avoid detection of reboot on startup
    _latestMulticastRebootFlag = true; // true to avoid detection of reboot on startup
}

} // namespace someip
