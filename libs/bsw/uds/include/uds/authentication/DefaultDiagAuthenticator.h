/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "uds/DiagReturnCode.h"
#include "uds/authentication/IDiagAuthenticator.h"

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
