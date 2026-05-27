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

#include "uds/authentication/IDiagAuthenticator.h"

#include <gmock/gmock.h>

namespace uds
{
using namespace ::testing;

class DiagAuthenticatorMock : public IDiagAuthenticator
{
public:
    MOCK_METHOD(bool, isAuthenticated, (uint16_t ecuId), (const, override));
    MOCK_METHOD(DiagReturnCode::Type, getNotAuthenticatedReturnCode, (), (const, override));
};

} // namespace uds
