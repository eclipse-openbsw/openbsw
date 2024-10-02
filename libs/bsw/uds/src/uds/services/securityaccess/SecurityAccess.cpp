// Copyright 2024 Accenture.

#include "uds/services/securityaccess/SecurityAccess.h"

#include "transport/TransportConfiguration.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"
#include "uds/session/ProgrammingSession.h"

#include <cstdlib>
#include <cstring>

namespace uds
{
using ::transport::TransportConfiguration;

SecurityAccess::SecurityAccess(DiagSession::DiagSessionMask const mask)
: Service(0x27U, mask)
, fSeed()
, fTesterId()
, fCurrentEcuId(static_cast<uint16_t>(TransportConfiguration::INVALID_DIAG_ADDRESS))
, fAuthenticatedEcu(static_cast<uint16_t>(TransportConfiguration::INVALID_DIAG_ADDRESS))
, fSecurityAccessType(0U)
{}

void SecurityAccess::resetAuthentication()
{
    fAuthenticatedEcu = static_cast<uint16_t>(TransportConfiguration::INVALID_DIAG_ADDRESS);
    fCurrentEcuId     = static_cast<uint16_t>(TransportConfiguration::INVALID_DIAG_ADDRESS);
}

bool SecurityAccess::isAuthenticated(uint16_t const address) const
{
    return (fAuthenticatedEcu == address);
}

DiagReturnCode::Type SecurityAccess::getNotAuthenticatedReturnCode() const
{
    return DiagReturnCode::ISO_SECURITY_ACCESS_DENIED;
}

uint8_t SecurityAccess::getSecurityAccessType() const { return fSecurityAccessType; }

} // namespace uds
