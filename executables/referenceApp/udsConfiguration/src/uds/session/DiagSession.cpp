// Copyright 2024 Accenture.
// Copyright 2026 BMW AG

#include "uds/session/DiagSession.h"

#include <uds/session/ApplicationDefaultSession.h>
#include <uds/session/ApplicationExtendedSession.h>
#include <uds/session/ProgrammingSession.h>

namespace uds
{
ApplicationDefaultSession& DiagSession::APPLICATION_DEFAULT_SESSION()
{
    static ApplicationDefaultSession applicationDefaultSession;
    return applicationDefaultSession;
}

ApplicationExtendedSession& DiagSession::APPLICATION_EXTENDED_SESSION()
{
    static ApplicationExtendedSession applicationExtendedSession;
    return applicationExtendedSession;
}

ProgrammingSession& DiagSession::PROGRAMMING_SESSION()
{
    static ProgrammingSession programmingSession;
    return programmingSession;
}

DiagSession::DiagSessionMask const& DiagSession::ALL_SESSIONS()
{
    static DiagSession::DiagSessionMask const allSessions
        = DiagSession::DiagSessionMask::getInstance().getOpenMask();
    return allSessions;
}

DiagSession::DiagSessionMask const& DiagSession::APPLICATION_EXTENDED_SESSION_MASK()
{
    return DiagSession::DiagSessionMask::getInstance()
           << DiagSession::APPLICATION_EXTENDED_SESSION();
}

bool operator==(DiagSession const& x, DiagSession const& y) { return x.toIndex() == y.toIndex(); }

bool operator!=(DiagSession const& x, DiagSession const& y) { return !(x == y); }

DiagSession::DiagSession(SessionType id, uint8_t index) : fType(id), fId(index) {}

// Default Session

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

// Extended Session

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

// Programming Session

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
