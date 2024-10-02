// Copyright 2024 Accenture.

#ifndef GUARD_2355B94A_39AE_4767_B23B_4F8992D4F8F6
#define GUARD_2355B94A_39AE_4767_B23B_4F8992D4F8F6

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

#endif // GUARD_2355B94A_39AE_4767_B23B_4F8992D4F8F6
