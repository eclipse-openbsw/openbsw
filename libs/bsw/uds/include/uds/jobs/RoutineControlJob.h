// Copyright 2024 Accenture.

#pragma once

#include "uds/base/AbstractDiagJob.h"

namespace uds
{
class RoutineControlJob : public AbstractDiagJob
{
protected:
    class RoutineControlJobNode;

public:
    RoutineControlJob(
        uint8_t const* const implementedRequest,
        uint8_t const implementedRequestLength,
        RoutineControlJobNode* const pStopRoutineJob,
        RoutineControlJobNode* const pRequestRoutineResultsJob,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS())
    : AbstractDiagJob(implementedRequest, implementedRequestLength, 2U, sessionMask)
    , fStarted(false)
    , fSequenceCheckEnabled(true)
    , fpStopRoutine(pStopRoutineJob)
    , fpRequestRoutineResults(pRequestRoutineResultsJob)
    {}

    RoutineControlJob(
        uint8_t const* const implementedRequest,
        uint8_t const implementedRequestLength,
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        RoutineControlJobNode* const pStopRoutineJob,
        RoutineControlJobNode* const pRequestRoutineResultsJob,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS())
    : AbstractDiagJob(
        implementedRequest,
        implementedRequestLength,
        2U,
        requestPayloadLength,
        responseLength,
        sessionMask)
    , fStarted(false)
    , fSequenceCheckEnabled(true)
    , fpStopRoutine(pStopRoutineJob)
    , fpRequestRoutineResults(pRequestRoutineResultsJob)
    {}

    virtual DiagReturnCode::Type
    start(IncomingDiagConnection& connection, uint8_t const* const request, uint16_t requestLength)
        = 0;

    virtual DiagReturnCode::Type stop(
        IncomingDiagConnection& /* connection */,
        uint8_t const* const /* request */,
        uint16_t const /* requestLength */)
    {
        return DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT;
    }

    virtual DiagReturnCode::Type requestResults(
        IncomingDiagConnection& /* connection */,
        uint8_t const* const /* request */,
        uint16_t const /* requestLength */)
    {
        return DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT;
    }

    AbstractDiagJob& getStartRoutine();

    AbstractDiagJob& getStopRoutine();

    AbstractDiagJob& getRequestRoutineResults();

    bool hasStopRoutine() { return nullptr != fpStopRoutine; }

    bool hasRequestResultsRoutine() { return nullptr != fpRequestRoutineResults; }

protected:
    class RoutineControlJobNode : public AbstractDiagJob
    {
    public:
        RoutineControlJobNode(
            uint8_t const* const implementedRequest,
            RoutineControlJob& routineControlJob,
            DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS())
        : AbstractDiagJob(implementedRequest, routineControlJob.getRequestLength(), 2U, sessionMask)
        , fRoutineControlJob(routineControlJob)
        {}

    private:
        DiagReturnCode::Type verify(uint8_t const* const request, uint16_t requestLength) override;
        RoutineControlJob& fRoutineControlJob;

        DiagReturnCode::Type process(
            IncomingDiagConnection& connection,
            uint8_t const* const request,
            uint16_t requestLength) override;
    };

    DiagReturnCode::Type verify(uint8_t const* const request, uint16_t requestLength) override;

    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const* const request,
        uint16_t requestLength) override;

    void disableSequenceCheck() { fSequenceCheckEnabled = false; }

    bool fStarted;
    bool fSequenceCheckEnabled;

private:
    friend class RoutineControlJobNode;

    RoutineControlJobNode* fpStopRoutine;
    RoutineControlJobNode* fpRequestRoutineResults;
};

} // namespace uds
