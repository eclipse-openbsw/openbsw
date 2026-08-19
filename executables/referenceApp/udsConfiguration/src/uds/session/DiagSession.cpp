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
// Session index of AppProgrammingSession. DiagSession::operator== compares
// only toIndex(), so this index decides which DiagnosticSessionControl paths
// treat the session as equal. It must be ApplicationExtendedSession's index
// (0x03): only sessions equal to APPLICATION_EXTENDED_SESSION() get the S3
// tester-present timeout armed (startTimeout()) and fall back to the default
// session when it expires (expired_local()). It must NOT be
// ProgrammingSession's index (0x04), whose switchSession() branch disables
// the UDS dispatcher for a bootloader handover, nor
// ApplicationDefaultSession's index (0x01), which would stop the timeout.
static constexpr uint8_t APP_PROGRAMMING_SESSION_INDEX = 0x03U;

// Application-level programming session. Uses SessionType PROGRAMMING (0x02)
// for the correct response byte, but shares the extended session's index so
// DiagnosticSessionControl keeps the UDS dispatcher alive and applies the
// extended session's S3 timeout handling. Jobs restricted to the extended
// session mask are consequently also allowed in this session.
class AppProgrammingSession : public DiagSession
{
public:
    AppProgrammingSession() : DiagSession(PROGRAMMING, APP_PROGRAMMING_SESSION_INDEX) {}

    DiagReturnCode::Type isTransitionPossible(DiagSession::SessionType targetSession) override;

    DiagSession& getTransitionResult(DiagSession::SessionType targetSession) override;
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
        {
            return DiagReturnCode::OK;
        }
        case DiagSession::PROGRAMMING:
        {
#ifdef PLATFORM_SUPPORT_PROGRAMMING_SESSION
            return DiagReturnCode::OK;
#else
            return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION;
#endif
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
// Application Programming Session

DiagReturnCode::Type
AppProgrammingSession::isTransitionPossible(DiagSession::SessionType const targetSession)
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
