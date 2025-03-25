// Copyright 2024 Accenture.

#pragma once

#include "uds/session/DiagSession.h"

namespace uds
{
class ProgrammingSession : public DiagSession
{
public:
    ProgrammingSession() : DiagSession(PROGRAMMING, 0x04U) {}

    DiagReturnCode::Type isTransitionPossible(DiagSession::SessionType targetSession) override;

    DiagSession& getTransitionResult(DiagSession::SessionType targetSession) override;
};

} // namespace uds

