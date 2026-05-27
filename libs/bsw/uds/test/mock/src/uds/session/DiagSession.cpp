/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/session/DiagSession.h"

#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/ProgrammingSession.h"

namespace uds
{
DiagSession::DiagSession(SessionType id, uint8_t index) : fType(id), fId(index) {}

DiagSession::DiagSessionMask const& DiagSession::ALL_SESSIONS()
{
    static DiagSession::DiagSessionMask const allSessions
        = DiagSession::DiagSessionMask::getInstance().getOpenMask();
    return allSessions;
}

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

DiagSession::DiagSessionMask const& DiagSession::APPLICATION_EXTENDED_SESSION_MASK()
{
    return DiagSession::DiagSessionMask::getInstance()
           << DiagSession::APPLICATION_EXTENDED_SESSION();
}

ProgrammingSession& DiagSession::PROGRAMMING_SESSION()
{
    static ProgrammingSession programmingSession;
    return programmingSession;
}

bool operator==(DiagSession const& x, DiagSession const& y) { return x.toIndex() == y.toIndex(); }

bool operator!=(DiagSession const& x, DiagSession const& y) { return !(x == y); }

} // namespace uds
