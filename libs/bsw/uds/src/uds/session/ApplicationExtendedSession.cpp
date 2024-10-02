// Copyright 2024 Accenture.

#include "uds/session/ApplicationExtendedSession.h"

#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/ProgrammingSession.h"

namespace uds
{

DiagReturnCode::Type
ApplicationExtendedSession::isTransitionPossible(DiagSession::SessionType const targetSession)
{
    switch (targetSession)
    {
        case DiagSession::DEFAULT:
        case DiagSession::EXTENDED:
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

DiagSession&
ApplicationExtendedSession::getTransitionResult(DiagSession::SessionType const targetSession)
{
    switch (targetSession)
    {
        case DiagSession::DEFAULT:
        {
            return DiagSession::APPLICATION_DEFAULT_SESSION();
        }
        case DiagSession::PROGRAMMING:
        {
            return DiagSession::PROGRAMMING_SESSION();
        }
        default:
        {
            return *this;
        }
    }
}

} // namespace uds
