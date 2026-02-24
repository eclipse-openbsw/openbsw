// Copyright 2025 Accenture.

#include "doip/server/DoIpServerVehicleIdentification.h"

namespace doip
{
DoIpServerVehicleIdentification::DoIpServerVehicleIdentification(
    GetVinCallback const getVinCallback,
    GetGidCallback const getGidCallback,
    GetEidCallback const getEidCallback,
    GetPowerModeCallback const getPowerModeCallback,
    OnVirReceivedCallback onVirReceivedCallback,
    ::estd::ordered_map<uint16_t, IDoIpUdpOemMessageHandler*> const* const oemMessageHandlers)
: IDoIpServerVehicleIdentificationCallback()
, _getVinCallback(getVinCallback)
, _getGidCallback(getGidCallback)
, _getEidCallback(getEidCallback)
, _getPowerModeCallback(getPowerModeCallback)
, _virReceivedCallback(onVirReceivedCallback)
, _oemMessageHandlers(oemMessageHandlers)
{}

void DoIpServerVehicleIdentification::getVin(VinType const vin) { _getVinCallback(vin); }

void DoIpServerVehicleIdentification::getGid(GidType const gid) { _getGidCallback(gid); }

void DoIpServerVehicleIdentification::getEid(EidType const eid) { _getEidCallback(eid); }

DoIpConstants::DiagnosticPowerMode DoIpServerVehicleIdentification::getPowerMode()
{
    return _getPowerModeCallback();
}

void DoIpServerVehicleIdentification::onVirReceived() { _virReceivedCallback(); }

IDoIpUdpOemMessageHandler*
DoIpServerVehicleIdentification::getOemMessageHandler(uint16_t payloadType) const
{
    if (_oemMessageHandlers == nullptr)
    {
        return nullptr;
    }
    auto const* const oemMessageHandler = _oemMessageHandlers->find(payloadType);
    if (oemMessageHandler == _oemMessageHandlers->end())
    {
        return nullptr;
    }
    // iterator checked beforehand
    return oemMessageHandler->second;
}

} // namespace doip
