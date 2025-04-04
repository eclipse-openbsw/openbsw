// Copyright 2024 Accenture.

#pragma once

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
