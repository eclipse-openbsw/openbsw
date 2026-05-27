/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup doip
 */
#pragma once

#include <platform/estdint.h>

namespace doip
{
/**
 * Interface for a DoIP client vehicle announcement listener.
 */
class IDoIpVehicleAnnouncementListener
{
public:
    /**
     * Should be called for each received vehicle announcement message.
     * \param logicalAddress logical address of the received target
     * \param localEndpoint reference to the local endpoint (destination endpoint)
     * \param remoteEndpoint reference to the device having sent to message
     */
    virtual void vehicleAnnouncementReceived(
        uint16_t logicalAddress,
        ::ip::IPEndpoint const& localEndpoint,
        ::ip::IPEndpoint const& remoteEndpoint)
        = 0;

protected:
    ~IDoIpVehicleAnnouncementListener()                                                  = default;
    IDoIpVehicleAnnouncementListener& operator=(IDoIpVehicleAnnouncementListener const&) = delete;
};

} // namespace doip
