// Copyright 2024 Accenture.

#ifndef GUARD_80BE143A_2041_4BE2_A208_D4D9EA5F399F
#define GUARD_80BE143A_2041_4BE2_A208_D4D9EA5F399F

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

#endif // GUARD_80BE143A_2041_4BE2_A208_D4D9EA5F399F
