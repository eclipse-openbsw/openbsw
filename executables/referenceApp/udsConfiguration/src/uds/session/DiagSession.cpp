/********************************************************************************
 * Copyright (c) 2024 Accenture
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/session/DiagSession.h"

#include <uds/session/ApplicationDefaultSession.h>
#include <uds/session/ApplicationExtendedSession.h>
#include <uds/session/ProgrammingSession.h>

namespace uds
{

#ifdef PLATFORM_SUPPORT_PROGRAMMING_SESSION
// Application-level programming session. Uses SessionType PROGRAMMING (0x02)
// for the correct response byte, but session index 0x02 instead of
// ProgrammingSession's 0x04 so switchSession()'s equality check against
// PROGRAMMING_SESSION() does not match.
class AppProgrammingSession : public DiagSession
{
public:
    AppProgrammingSession() : DiagSession(PROGRAMMING, 0x02U) {}

    DiagReturnCode::Type isTransitionPossible(SessionType const target) override
    {
        switch (target)
        {
            case DEFAULT:
            case EXTENDED:
            case PROGRAMMING: return DiagReturnCode::OK;
            default:          return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
        }
    }

    DiagSession& getTransitionResult(SessionType target) override;
};

static AppProgrammingSession& appProgrammingSession()
{
    static AppProgrammingSession s;
    return s;
}
#endif

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
#ifdef PLATFORM_SUPPORT_PROGRAMMING_SESSION
        case DiagSession::PROGRAMMING:
#endif
        {
            return DiagReturnCode::OK;
        }
#ifndef PLATFORM_SUPPORT_PROGRAMMING_SESSION
        case DiagSession::PROGRAMMING:
        {
            return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION;
        }
#endif
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
#ifdef PLATFORM_SUPPORT_PROGRAMMING_SESSION
        case DiagSession::PROGRAMMING:
        {
            return appProgrammingSession();
        }
#endif
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
#ifdef PLATFORM_SUPPORT_PROGRAMMING_SESSION
            return appProgrammingSession();
#else
            return DiagSession::PROGRAMMING_SESSION();
#endif
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

#ifdef PLATFORM_SUPPORT_PROGRAMMING_SESSION
DiagSession&
AppProgrammingSession::getTransitionResult(DiagSession::SessionType const targetSession)
{
    switch (targetSession)
    {
        case DiagSession::DEFAULT:
        {
            return DiagSession::APPLICATION_DEFAULT_SESSION();
        }
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
#endif

} // namespace uds
