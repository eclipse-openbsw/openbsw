// Copyright 2024 Accenture.

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

} // namespace uds
