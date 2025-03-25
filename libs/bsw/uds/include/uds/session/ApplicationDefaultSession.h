// Copyright 2024 Accenture.

#pragma once

#include "uds/session/DiagSession.h"

namespace uds
{
class ApplicationDefaultSession : public DiagSession
{
public:
    ApplicationDefaultSession() : DiagSession(DEFAULT, 0x01U) {}

    DiagReturnCode::Type isTransitionPossible(DiagSession::SessionType targetSession) override;

    DiagSession& getTransitionResult(DiagSession::SessionType targetSession) override;
};

} /* namespace uds */
