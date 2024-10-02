// Copyright 2024 Accenture.

#include "uds/services/writedata/WriteDataByIdentifier.h"

#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"

namespace uds
{
WriteDataByIdentifier::WriteDataByIdentifier()
: Service(ServiceId::WRITE_DATA_BY_IDENTIFIER, DiagSession::ALL_SESSIONS())
{
    setDefaultDiagReturnCode(DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE);
}

DiagReturnCode::Type
WriteDataByIdentifier::verify(uint8_t const* const request, uint16_t const requestLength)
{
    DiagReturnCode::Type result = Service::verify(request, requestLength);
    if (result == DiagReturnCode::OK)
    {
        if (requestLength < 3U)
        {
            result = DiagReturnCode::ISO_INVALID_FORMAT;
        }
    }
    return result;
}

} // namespace uds
