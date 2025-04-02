// Copyright 2024 Accenture.

#include "uds/services/controldtcsetting/ControlDTCSetting.h"

#include "uds/UdsLogger.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"

namespace uds
{
using ::util::logger::Logger;
using ::util::logger::UDS;

ControlDTCSetting::ControlDTCSetting()
: Service(
    ServiceId::CONTROL_DTC_SETTING,
    EXPECTED_REQUEST_LENGTH,
    RESPONSE_LENGTH,
    DiagSessionMask::getInstance() << DiagSession::APPLICATION_EXTENDED_SESSION())
{
    enableSuppressPositiveResponse();
}

DiagReturnCode::Type ControlDTCSetting::process(
    IncomingDiagConnection& connection,
    uint8_t const* const request,
    uint16_t const /* requestLength */)
{
    uint8_t const dtcSettingType = request[0];
    Logger::debug(UDS, "ControlDTCSetting %d", dtcSettingType);
    switch (dtcSettingType)
    {
        case CONTROL_DTC_SETTING_ON:
        {
            // Allow creation of new DTCs. Intentionally not implemented.
            break;
        }
        case CONTROL_DTC_SETTING_OFF:
        {
            // Disallow creation of new DTCs. Intentionally not implemented.
            break;
        }
        default:
        {
            return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
        }
    }
    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(dtcSettingType);
    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

} // namespace uds
