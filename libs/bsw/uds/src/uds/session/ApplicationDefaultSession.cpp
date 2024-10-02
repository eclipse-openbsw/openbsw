// Copyright 2024 Accenture.

#include "uds/session/ApplicationDefaultSession.h"

#include "uds/session/ApplicationExtendedSession.h"

namespace uds
{

DiagReturnCode::Type
ApplicationDefaultSession::isTransitionPossible(DiagSession::SessionType const targetSession)
{
    switch (targetSession)
    {
        case DiagSession::DEFAULT:
        case DiagSession::EXTENDED:
        {
            return DiagReturnCode::OK;
        }
        case DiagSession::PROGRAMMING:
        {
            return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION;
        }
        default:
        {
            return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
        }
    }
}

DiagSession&
ApplicationDefaultSession::getTransitionResult(DiagSession::SessionType const targetSession)
{
    switch (targetSession)
    {
        case DiagSession::EXTENDED:
        {
            return DiagSession::APPLICATION_EXTENDED_SESSION();
        }
        default:
        {
            return *this;
        }
    }
}

} // namespace uds
