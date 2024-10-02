// Copyright 2024 Accenture.

#ifndef GUARD_8CA4486A_11C2_4A3A_81F3_E3930FB7596F
#define GUARD_8CA4486A_11C2_4A3A_81F3_E3930FB7596F

#include "uds/services/sessioncontrol/ISessionPersistence.h"

#include <gmock/gmock.h>

namespace uds
{
class SessionPersistenceMock : public ISessionPersistence
{
public:
    SessionPersistenceMock() {}

    MOCK_METHOD1(readSession, void(DiagnosticSessionControl& sessionControl));
    MOCK_METHOD2(writeSession, void(DiagnosticSessionControl& sessionControl, uint8_t session));
};

} // namespace uds

#endif // GUARD_8CA4486A_11C2_4A3A_81F3_E3930FB7596F
