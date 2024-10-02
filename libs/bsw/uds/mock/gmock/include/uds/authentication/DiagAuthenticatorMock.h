// Copyright 2024 Accenture.

#ifndef GUARD_377BD233_E0B5_4DBA_89B4_D54DBE2BBE14
#define GUARD_377BD233_E0B5_4DBA_89B4_D54DBE2BBE14

#include "uds/authentication/IDiagAuthenticator.h"

#include <gmock/gmock.h>

namespace uds
{
using namespace ::testing;

class DiagAuthenticatorMock : public IDiagAuthenticator
{
public:
    MOCK_METHOD(bool, isAuthenticated, (uint16_t ecuId), (const));
    MOCK_METHOD(DiagReturnCode::Type, getNotAuthenticatedReturnCode, (), (const));
};

} // namespace uds

#endif // GUARD_377BD233_E0B5_4DBA_89B4_D54DBE2BBE14
