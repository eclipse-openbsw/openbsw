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

#include "doip/server/IDoIpServerVehicleIdentificationCallback.h"
#include "doip/server/IDoIpUdpOemMessageHandler.h"

#include <etl/delegate.h>
#include <etl/map.h>

namespace doip
{
/**
 * Callback based implementation for accessing vehicle identification data.
 */
class DoIpServerVehicleIdentification : public IDoIpServerVehicleIdentificationCallback
{
public:
    /** Fills a given span with the vehicle's VIN. */
    using GetVinCallback = ::etl::delegate<void(VinType)>;

    /** Fills a given span with the DoIP group ID. */
    using GetGidCallback = ::etl::delegate<void(GidType)>;

    /** Fills a given span with the DoIP entity ID. */
    using GetEidCallback = ::etl::delegate<void(EidType)>;

    /** Returns current diagnostic power mode. */
    using GetPowerModeCallback = ::etl::delegate<DoIpConstants::DiagnosticPowerMode()>;

    /** Callback whenever a Vehicle Identification Request has been received. */
    using OnVirReceivedCallback = ::etl::delegate<void()>;

    /**
     * Constructor.
     * \param getVinCallback callback function that will be called to retrieve the VIN
     * \param getGidCallback callback function that will be called to retrieve the GID
     * \param getEidCallback callback function that will be called to retrieve the EID
     * \param getPowerModeCallback callback function that will be called to retrieve the power mode
     * \param virReceivedCallback callback function that will be called when a Vehicle
     * Identification Request has been received \param oemMessageHandlers dispatch map for handling
     * OEM custom messages. Parameter is nullable, if user is not interested in handling OEM
     * extensions to ISO
     */
    DoIpServerVehicleIdentification(
        GetVinCallback getVinCallback,
        GetGidCallback getGidCallback,
        GetEidCallback getEidCallback,
        GetPowerModeCallback getPowerModeCallback,
        OnVirReceivedCallback onVirReceivedCallback,
        ::etl::imap<uint16_t, IDoIpUdpOemMessageHandler*> const* const oemMessageHandlers);

    void getVin(VinType vin) override;
    void getGid(GidType gid) override;
    void getEid(EidType eid) override;
    DoIpConstants::DiagnosticPowerMode getPowerMode() override;
    void onVirReceived() override;
    IDoIpUdpOemMessageHandler* getOemMessageHandler(uint16_t payloadType) const override;

private:
    GetVinCallback _getVinCallback;
    GetGidCallback _getGidCallback;
    GetEidCallback _getEidCallback;
    GetPowerModeCallback _getPowerModeCallback;
    OnVirReceivedCallback _virReceivedCallback;
    ::etl::imap<uint16_t, IDoIpUdpOemMessageHandler*> const* const _oemMessageHandlers;
};

} // namespace doip
