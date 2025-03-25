// Copyright 2024 Accenture.

#pragma once

#include "uds/session/DiagSession.h"

namespace uds
{
class ApplicationExtendedSession : public DiagSession
{
public:
    ApplicationExtendedSession() : DiagSession(EXTENDED, 0x03U) {}

    DiagReturnCode::Type isTransitionPossible(DiagSession::SessionType targetSession) override;

    DiagSession& getTransitionResult(DiagSession::SessionType targetSession) override;
};

} // namespace uds

