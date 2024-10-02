// Copyright 2024 Accenture.

#include "uds/base/DiagJobRoot.h"

#include "transport/TransportConfiguration.h"
#include "uds/UdsConstants.h"
#include "uds/authentication/DefaultDiagAuthenticator.h"
#include "uds/authentication/IDiagAuthenticator.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/IDiagSessionManager.h"

namespace uds
{
using ::transport::TransportConfiguration;

DiagJobRoot::DiagJobRoot() : AbstractDiagJob(nullptr, 0U, 0U)
{
    fDefaultDiagReturnCode = DiagReturnCode::ISO_SERVICE_NOT_SUPPORTED;
    sfpDiagJobRoot         = this;
}

DiagJobRoot::~DiagJobRoot() { sfpDiagJobRoot = nullptr; }

DiagReturnCode::Type DiagJobRoot::verify(uint8_t const* const request, uint16_t const requestLength)
{
    if ((request == nullptr) || (requestLength < 1U))
    { // no empty requests!
        return DiagReturnCode::ISO_GENERAL_REJECT;
    }
    return DiagReturnCode::OK;
}

DiagReturnCode::Type DiagJobRoot::execute(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    if (connection.fServiceId == 0x7FU) // no response to incoming NRC
    {
        connection.terminate();
        return DiagReturnCode::OK;
    }
    if (TransportConfiguration::isFunctionalAddress(connection.fTargetId))
    {
        if (connection.fServiceId == uds::ServiceId::TESTER_PRESENT)
        {
            if ((!getDiagSessionManager().isSessionTimeoutActive()) && (requestLength > 1U)
                && (((request[1] & SUPPRESS_POSITIVE_RESPONSE_MASK)) > 0U))
            {
                connection.terminate();
                return DiagReturnCode::OK;
            }
        }
    }
    DiagReturnCode::Type const ret = verify(request, requestLength);
    if (ret != DiagReturnCode::OK)
    {
        (void)getDiagSessionManager().acceptedJob(connection, *this, request, requestLength);
        return ret;
    }
    if (!fAllowedSessions.match(getSession()))
    {
        acceptJob(connection, request, requestLength);
        return DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE;
    }
    return process(connection, request, requestLength);
}

DiagReturnCode::Type
DiagJobRoot::verifySupplierIndication(uint8_t const* const request, uint16_t const requestLength)
{
    (void)request;
    (void)requestLength;
    return DiagReturnCode::OK;
}

} // namespace uds
