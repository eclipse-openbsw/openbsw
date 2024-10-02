// Copyright 2024 Accenture.

#ifndef GUARD_13AD7767_9D80_45F3_9E9B_806DB012BC38
#define GUARD_13AD7767_9D80_45F3_9E9B_806DB012BC38

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

#endif // GUARD_13AD7767_9D80_45F3_9E9B_806DB012BC38
