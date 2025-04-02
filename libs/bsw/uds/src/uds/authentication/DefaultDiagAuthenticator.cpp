// Copyright 2024 Accenture.

#include "uds/authentication/DefaultDiagAuthenticator.h"

namespace uds
{
bool DefaultDiagAuthenticator::isAuthenticated(uint16_t const /* address */) const { return true; }

DiagReturnCode::Type DefaultDiagAuthenticator::getNotAuthenticatedReturnCode() const
{
    return DiagReturnCode::ISO_AUTHENTICATION_REQUIRED;
}

} // namespace uds
