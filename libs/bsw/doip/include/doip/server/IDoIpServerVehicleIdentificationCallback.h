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

#include "doip/common/DoIpConstants.h"
#include "doip/server/IDoIpUdpOemMessageHandler.h"

#include <etl/span.h>

namespace doip
{
/**
 * Callback interface that allows simple access to the vehicle identification data.
 */
class IDoIpServerVehicleIdentificationCallback
{
public:
    /** Type that encapsulates a VIN */
    using VinType = ::etl::span<char, 17>;

    /** Type that encapsulates a GID */
    using GidType = ::etl::span<uint8_t, 6>;

    /** Type that encapsulates a EID */
    using EidType = ::etl::span<uint8_t, 6>;

    /**
     * Get the current value of the VIN.
     * \param vin span that will receive the VIN value
     */
    virtual void getVin(VinType vin) = 0;

    /**
     * Get the current value of the GID.
     * \param gid span that will receive the GID value
     */
    virtual void getGid(GidType gid) = 0;

    /**
     * Get the current value of the EID.
     * \param eid span that will receive the EID value
     */
    virtual void getEid(EidType eid) = 0;

    /**
     * Get the current power mode value.
     * \return power mode
     */
    virtual DoIpConstants::DiagnosticPowerMode getPowerMode() = 0;

    /**
     * Notify the implementation that a Vehicle Identification Request has been received.
     */
    virtual void onVirReceived() = 0;

    /**
     * Get the handler for a manufacturer specific request
     */
    virtual IDoIpUdpOemMessageHandler* getOemMessageHandler(uint16_t payloadType) const = 0;

protected:
    ~IDoIpServerVehicleIdentificationCallback() = default;
    IDoIpServerVehicleIdentificationCallback&
    operator=(IDoIpServerVehicleIdentificationCallback const&)
        = default;
};

} // namespace doip
