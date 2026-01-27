// Copyright 2024 Accenture.

#include "uds/session/ProgrammingSession.h"

#include "uds/session/ApplicationDefaultSession.h"

namespace uds
{

DiagReturnCode::Type
ProgrammingSession::isTransitionPossible(DiagSession::SessionType const targetSession)
{
    switch (targetSession)
    {
        case DiagSession::DEFAULT:
        case DiagSession::PROGRAMMING:
        {
            return DiagReturnCode::OK;
        }
        default:
        {
            return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
        }
    }
}

DiagSession& ProgrammingSession::getTransitionResult(DiagSession::SessionType const targetSession)
{
    switch (targetSession)
    {
        case DiagSession::DEFAULT:
        {
            return DiagSession::APPLICATION_DEFAULT_SESSION();
        }
        default:
        {
            return *this;
        }
    }
}

} // namespace uds
