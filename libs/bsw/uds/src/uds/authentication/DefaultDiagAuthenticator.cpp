/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/authentication/DefaultDiagAuthenticator.h"

namespace uds
{
bool DefaultDiagAuthenticator::isAuthenticated(uint16_t const /* address */) const { return true; }

DiagReturnCode::Type DefaultDiagAuthenticator::getNotAuthenticatedReturnCode() const
{
    return DiagReturnCode::ISO_AUTHENTICATION_REQUIRED;
}

} // namespace uds
