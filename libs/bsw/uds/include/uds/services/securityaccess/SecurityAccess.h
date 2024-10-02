// Copyright 2024 Accenture.

#ifndef GUARD_872C2BA9_B0E2_4EC9_9B51_801211F37B68
#define GUARD_872C2BA9_B0E2_4EC9_9B51_801211F37B68

#include "uds/authentication/DefaultDiagAuthenticator.h"
#include "uds/authentication/IDiagAuthenticator.h"
#include "uds/base/Service.h"

namespace uds
{
class SecurityAccess
: public uds::IDiagAuthenticator
, public Service
{
public:
    explicit SecurityAccess(DiagSession::DiagSessionMask mask);

    /**
     * \see IDiagAuthenticator::isAuthenticated()
     */
    bool isAuthenticated(uint16_t address) const override;
    /**
     * \see IDiagAuthenticator::getNotAuthenticatedReturnCode()
     */
    DiagReturnCode::Type getNotAuthenticatedReturnCode() const override;

    void resetAuthentication();

    /**
     * \return securityaccesstype
     */
    uint8_t getSecurityAccessType() const;

protected:
    static uint8_t const RANDOM_LENGTH    = 8U;
    static uint8_t const TESTER_ID_LENGTH = 4U;
    uint8_t fSeed[RANDOM_LENGTH];
    uint8_t fTesterId[TESTER_ID_LENGTH];
    uint16_t fCurrentEcuId;
    uint16_t fAuthenticatedEcu;
    uint8_t fSecurityAccessType;
};

} // namespace uds

#endif // GUARD_872C2BA9_B0E2_4EC9_9B51_801211F37B68
