/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "doip/common/DoIpConstants.h"

#include <platform/estdint.h>

namespace doip
{
/**
 * Interface to DoIp server vehicle announcement wait and interval parameters
 * ISO 13400: A_DoIP_Announce_Wait, A_DoIP_Announce_Interval
 */
class IDoIpServerVehicleAnnouncementParameterProvider
{
public:
    /**
     * This timing parameter specifies the initial time that a DoIP entity waits until it responds
     * to a vehicle identification request and the time that a DoIP entity waits until it transmits
     * a vehicle announcement message after a valid IP address is configured. \return timeout in
     * milliseconds
     */
    virtual uint16_t getAnnounceWait() const = 0;

    /**
     * This timing parameter specifies the time between the vehicle announcement messages that are
     * sent by the DoIP entities after a valid IP address has been configured. \return timeout in
     * milliseconds
     */

    virtual uint16_t getAnnounceInterval() const
    {
        return doip::DoIpConstants::Timings::DOIP_ANNOUNCE_INTERVAL_MS;
    }

protected:
    ~IDoIpServerVehicleAnnouncementParameterProvider() = default;
    IDoIpServerVehicleAnnouncementParameterProvider&
    operator=(IDoIpServerVehicleAnnouncementParameterProvider const&)
        = default;
};

} // namespace doip
