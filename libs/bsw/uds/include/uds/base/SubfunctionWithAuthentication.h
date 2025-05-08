// Copyright 2024 Accenture.

#pragma once

#include "uds/base/Subfunction.h"

namespace uds
{
class IDiagAuthenticator;

/**
 * Base class for UDS subfunctions
 *
 */
class SubfunctionWithAuthentication : public Subfunction
{
public:
    SubfunctionWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint8_t const implementedRequest[],
        DiagSession::DiagSessionMask sessionMask);

    SubfunctionWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint8_t const implementedRequest[],
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSession::DiagSessionMask sessionMask);

protected:
    /**
     * \see AbstractDiagJob::getDiagAuthenticator()
     */
    IDiagAuthenticator const& getDiagAuthenticator() const override;

private:
    IDiagAuthenticator const& fDiagAuthenticator;
};

} // namespace uds
