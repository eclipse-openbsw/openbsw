// Copyright 2024 Accenture.

#pragma once

#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/IDiagSessionChangedListener.h"
#include "uds/session/IDiagSessionManager.h"

#include <gmock/gmock.h>

namespace uds
{
class DiagSessionManagerMock : public IDiagSessionManager
{
public:
    MOCK_METHOD(DiagSession const&, getActiveSession, (), (const, override));
    MOCK_METHOD(void, startSessionTimeout, (), (override));
    MOCK_METHOD(void, stopSessionTimeout, (), (override));
    MOCK_METHOD(bool, isSessionTimeoutActive, (), (override));
    MOCK_METHOD(void, resetToDefaultSession, (), (override));
    MOCK_METHOD(bool, persistAndRestoreSession, (), (override));

    MOCK_METHOD(
        DiagReturnCode::Type,
        acceptedJob,
        (IncomingDiagConnection&, AbstractDiagJob const&, uint8_t const[], uint16_t),
        (override));
    MOCK_METHOD(
        void,
        responseSent,
        (IncomingDiagConnection&, DiagReturnCode::Type, uint8_t const[], uint16_t),
        (override));

    MOCK_METHOD(void, addDiagSessionListener, (IDiagSessionChangedListener&), (override));
    MOCK_METHOD(void, removeDiagSessionListener, (IDiagSessionChangedListener&), (override));
};

} // namespace uds
