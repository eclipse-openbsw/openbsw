// Copyright 2024 Accenture.

#ifndef GUARD_3D447BA4_4395_4FD9_B58B_D80FC0EFBDCA
#define GUARD_3D447BA4_4395_4FD9_B58B_D80FC0EFBDCA

#include "uds/connection/OutgoingDiagConnection.h"

#include <gmock/gmock.h>

namespace uds
{
using namespace ::testing;

class OutgoingDiagConnectionMock : public OutgoingDiagConnection
{
public:
    MOCK_METHOD0(terminate, void());
    MOCK_METHOD0(timeoutOccurred, void());
};

} // namespace uds

#endif // GUARD_3D447BA4_4395_4FD9_B58B_D80FC0EFBDCA
