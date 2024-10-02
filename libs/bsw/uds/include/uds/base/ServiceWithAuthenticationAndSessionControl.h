// Copyright 2024 Accenture.

#ifndef GUARD_C9271682_6316_4C35_B41E_511F17107E49
#define GUARD_C9271682_6316_4C35_B41E_511F17107E49

#include "estd/uncopyable.h"
#include "uds/base/ServiceWithAuthentication.h"

namespace uds
{
class IDiagSessionManager;

/**
 * Helper base class for UDS services
 *
 *
 * \see Service
 */
class ServiceWithAuthenticationAndSessionControl : public ServiceWithAuthentication
{
    UNCOPYABLE(ServiceWithAuthenticationAndSessionControl);

public:
    explicit ServiceWithAuthenticationAndSessionControl(
        IDiagAuthenticator const& authenticator,
        IDiagSessionManager& sessionManager,
        uint8_t const service,
        DiagSession::DiagSessionMask sessionMask);

    explicit ServiceWithAuthenticationAndSessionControl(
        IDiagAuthenticator const& authenticator,
        IDiagSessionManager& sessionManager,
        uint8_t const service,
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

#endif // GUARD_C9271682_6316_4C35_B41E_511F17107E49
