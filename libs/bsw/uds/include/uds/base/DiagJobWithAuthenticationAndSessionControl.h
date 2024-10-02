// Copyright 2024 Accenture.

#ifndef GUARD_53A9846E_21F7_4F26_B2FF_4CF80D17637B
#define GUARD_53A9846E_21F7_4F26_B2FF_4CF80D17637B

#include "estd/uncopyable.h"
#include "uds/base/DiagJobWithAuthentication.h"

namespace uds
{
class IDiagSessionManager;

class DiagJobWithAuthenticationAndSessionControl : public DiagJobWithAuthentication
{
    UNCOPYABLE(DiagJobWithAuthenticationAndSessionControl);

public:
    explicit DiagJobWithAuthenticationAndSessionControl(
        IDiagAuthenticator const& authenticator,
        IDiagSessionManager& sessionManager,
        uint8_t const implementedRequest[],
        uint8_t const requestLength,
        uint8_t const prefixLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS());

    explicit DiagJobWithAuthenticationAndSessionControl(
        IDiagAuthenticator const& authenticator,
        IDiagSessionManager& sessionManager,
        uint8_t const implementedRequest[],
        uint8_t const requestLength,
        uint8_t const prefixLength,
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS());

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

#endif // GUARD_53A9846E_21F7_4F26_B2FF_4CF80D17637B
