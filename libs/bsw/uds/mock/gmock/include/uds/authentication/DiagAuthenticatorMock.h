// Copyright 2024 Accenture.

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
