// Copyright 2024 Accenture.

#pragma once

#include "uds/DiagReturnCode.h"
#include "uds/authentication/IDiagAuthenticator.h"

#include <estd/uncopyable.h>

namespace uds
{
/**
 * Subclass of IDiagAuthenticator which will always return true for
 * isAuthenticated().
 *
 *
 * This class should be used by diagnosis jobs which do not need
 * authentication.
 */
class DefaultDiagAuthenticator : public IDiagAuthenticator
{
    UNCOPYABLE(DefaultDiagAuthenticator);

public:
    DefaultDiagAuthenticator() = default;

    /**
     * Always returns true
     * \see IDiagAuthenticator::isAuthenticated()
     */
    bool isAuthenticated(uint16_t address) const override;
    /**
     * \see IDiagAuthenticator::getNotAuthenticatedReturnCode()
     */
    DiagReturnCode::Type getNotAuthenticatedReturnCode() const override;
};

} // namespace uds

