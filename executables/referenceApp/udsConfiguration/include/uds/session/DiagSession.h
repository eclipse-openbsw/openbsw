// Copyright 2024 Accenture.

#pragma once

#include <uds/DiagReturnCode.h>
#include <util/Mask.h>

namespace uds
{
class ApplicationDefaultSession;
class ApplicationExtendedSession;
class ProgrammingSession;

class DiagSession
{
public:
    enum
    {
        MAX_INDEX = 8
    };

    typedef Mask<DiagSession> DiagSessionMask;

    static ApplicationDefaultSession& APPLICATION_DEFAULT_SESSION();
    static ApplicationExtendedSession& APPLICATION_EXTENDED_SESSION();
    static ProgrammingSession& PROGRAMMING_SESSION();

    static DiagSessionMask const& ALL_SESSIONS();
    static DiagSessionMask const& APPLICATION_EXTENDED_SESSION_MASK();

    enum SessionType
    {
        DEFAULT     = 0x01,
        PROGRAMMING = 0x02,
        EXTENDED    = 0x03,
    };

    uint8_t toIndex() const { return fId; }

    uint8_t getSessionByte() const { return fType; }

    SessionType getType() const { return fType; }

    virtual DiagReturnCode::Type isTransitionPossible(DiagSession::SessionType targetSession) = 0;

    virtual DiagSession& getTransitionResult(DiagSession::SessionType targetSession) = 0;

    virtual void enter() {}

protected:
    DiagSession(SessionType id, uint8_t index);

private:
    SessionType fType;
    uint8_t fId;
};

bool operator==(DiagSession const& x, DiagSession const& y);
bool operator!=(DiagSession const& x, DiagSession const& y);

} // namespace uds
