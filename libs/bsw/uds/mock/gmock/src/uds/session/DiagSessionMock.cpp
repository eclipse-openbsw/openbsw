// Copyright 2024 Accenture.

#include "uds/session/DiagSessionMock.h"

#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/DiagSession.h"
#include "uds/session/ProgrammingSession.h"

namespace uds
{

DiagReturnCode::Type
ProgrammingSession::isTransitionPossible(DiagSession::SessionType const targetSession)
{
    if (DiagSessionMockHelper<ProgrammingSession>::instance().isStub())
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
    return DiagSessionMockHelper<ProgrammingSession>::instance().isTransitionPossible(
        targetSession);
}

DiagSession& ProgrammingSession::getTransitionResult(DiagSession::SessionType const targetSession)
{
    if (DiagSessionMockHelper<ProgrammingSession>::instance().isStub())
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
    return DiagSessionMockHelper<ProgrammingSession>::instance().getTransitionResult(targetSession);
}

DiagReturnCode::Type
ApplicationDefaultSession::isTransitionPossible(DiagSession::SessionType const targetSession)
{
    if (DiagSessionMockHelper<ApplicationDefaultSession>::instance().isStub())
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
    return DiagSessionMockHelper<ApplicationDefaultSession>::instance().isTransitionPossible(
        targetSession);
}

DiagSession&
ApplicationDefaultSession::getTransitionResult(DiagSession::SessionType const targetSession)
{
    if (DiagSessionMockHelper<ApplicationDefaultSession>::instance().isStub())
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
    return DiagSessionMockHelper<ApplicationDefaultSession>::instance().getTransitionResult(
        targetSession);
}

DiagReturnCode::Type
ApplicationExtendedSession::isTransitionPossible(DiagSession::SessionType const targetSession)
{
    if (DiagSessionMockHelper<ApplicationExtendedSession>::instance().isStub())
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
    return DiagSessionMockHelper<ApplicationExtendedSession>::instance().isTransitionPossible(
        targetSession);
}

DiagSession&
ApplicationExtendedSession::getTransitionResult(DiagSession::SessionType const targetSession)
{
    if (DiagSessionMockHelper<ApplicationExtendedSession>::instance().isStub())
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
    return DiagSessionMockHelper<ApplicationExtendedSession>::instance().getTransitionResult(
        targetSession);
}
} // namespace uds
