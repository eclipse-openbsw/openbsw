// Copyright 2024 Accenture.

#pragma once

#include "estd/uncopyable.h"
#include "uds/base/SubfunctionWithAuthentication.h"

namespace uds
{
class IDiagSessionManager;

/**
 * Base class for UDS subfunctions
 *
 */
class SubfunctionWithAuthenticationAndSessionControl : public SubfunctionWithAuthentication
{
    UNCOPYABLE(SubfunctionWithAuthenticationAndSessionControl);

public:
    SubfunctionWithAuthenticationAndSessionControl(
        IDiagAuthenticator const& authenticator,
        IDiagSessionManager& sessionManager,
        uint8_t const implementedRequest[],
        DiagSession::DiagSessionMask sessionMask);

    SubfunctionWithAuthenticationAndSessionControl(
        IDiagAuthenticator const& authenticator,
        IDiagSessionManager& sessionManager,
        uint8_t const implementedRequest[],
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSession::DiagSessionMask sessionMask);

protected:
    /**
     * \see AbstractDiagJob::getDiagSessionManager()
     */
    IDiagSessionManager& getDiagSessionManager() override;

    /**
     * \see AbstractDiagJob::getDiagSessionManager()
     */
    IDiagSessionManager const& getDiagSessionManager() const override;

private:
    IDiagSessionManager& fSessionManager;
};

} // namespace uds

