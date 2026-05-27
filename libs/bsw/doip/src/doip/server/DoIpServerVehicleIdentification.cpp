/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/server/DoIpServerVehicleIdentification.h"

namespace doip
{
DoIpServerVehicleIdentification::DoIpServerVehicleIdentification(
    GetVinCallback const getVinCallback,
    GetGidCallback const getGidCallback,
    GetEidCallback const getEidCallback,
    GetPowerModeCallback const getPowerModeCallback,
    OnVirReceivedCallback onVirReceivedCallback,
    ::etl::imap<uint16_t, IDoIpUdpOemMessageHandler*> const* const oemMessageHandlers)
: IDoIpServerVehicleIdentificationCallback()
, _getVinCallback(getVinCallback)
, _getGidCallback(getGidCallback)
, _getEidCallback(getEidCallback)
, _getPowerModeCallback(getPowerModeCallback)
, _virReceivedCallback(onVirReceivedCallback)
, _oemMessageHandlers(oemMessageHandlers)
{}

void DoIpServerVehicleIdentification::getVin(VinType const vin)
{
    if (_getVinCallback.is_valid())
    {
        _getVinCallback(vin);
    }
}

void DoIpServerVehicleIdentification::getGid(GidType const gid)
{
    if (_getGidCallback.is_valid())
    {
        _getGidCallback(gid);
    }
}

void DoIpServerVehicleIdentification::getEid(EidType const eid)
{
    if (_getEidCallback.is_valid())
    {
        _getEidCallback(eid);
    }
}

DoIpConstants::DiagnosticPowerMode DoIpServerVehicleIdentification::getPowerMode()
{
    return _getPowerModeCallback();
}

void DoIpServerVehicleIdentification::onVirReceived() { _virReceivedCallback(); }

IDoIpUdpOemMessageHandler*
DoIpServerVehicleIdentification::getOemMessageHandler(uint16_t const payloadType) const
{
    if (_oemMessageHandlers == nullptr)
    {
        return nullptr;
    }
    auto const oemMessageHandler = _oemMessageHandlers->find(payloadType);
    if (oemMessageHandler == _oemMessageHandlers->end())
    {
        return nullptr;
    }
    // iterator checked beforehand
    return oemMessageHandler->second;
}

} // namespace doip
