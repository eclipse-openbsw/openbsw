/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <cstdint>

namespace someip
{
class RebootTrackerEndpoint
{
public:
    RebootTrackerEndpoint();

    /**
     * Sets latest unicast session ID and latest unicast reboot flag to passed values.
     */
    void setUnicast(uint16_t sessionId, bool rebootFlag);

    /**
     * Sets latest multicast session ID and latest multicast reboot flag to passed
     * values.
     */
    void setMulticast(uint16_t sessionId, bool rebootFlag);

    /**
     * TODO.
     */
    bool getMulticastRebootValue(uint16_t sessionId, bool rebootFlag) const;

    /**
     * TODO.
     */
    bool getUnicastRebootValue(uint16_t sessionId, bool rebootFlag) const;

    /**
     * lifecycle method that initializes latest unicast session ID, latest multicast session ID,
     * latest unicast reboot flag and latest multicast reboot flag.
     */
    void initActiveReboot();

private:
    uint16_t _latestUnicastSessionId;
    uint16_t _latestMulticastSessionId;
    bool _latestUnicastRebootFlag;
    bool _latestMulticastRebootFlag;
};

} // namespace someip
