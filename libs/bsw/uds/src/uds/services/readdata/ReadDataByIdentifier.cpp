// Copyright 2024 Accenture.

#include "uds/services/readdata/ReadDataByIdentifier.h"

#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"

namespace uds
{
ReadDataByIdentifier::ReadDataByIdentifier()
: Service(ServiceId::READ_DATA_BY_IDENTIFIER, DiagSession::ALL_SESSIONS())
{
    setDefaultDiagReturnCode(DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE);
}

DiagReturnCode::Type
ReadDataByIdentifier::verify(uint8_t const* const request, uint16_t const requestLength)
{
    DiagReturnCode::Type result = Service::verify(request, requestLength);
    if (result == DiagReturnCode::OK)
    {
        if (requestLength != EXPECTED_REQUEST_LENGTH)
        {
            result = DiagReturnCode::ISO_INVALID_FORMAT;
        }
    }
    return result;
}

} // namespace uds
