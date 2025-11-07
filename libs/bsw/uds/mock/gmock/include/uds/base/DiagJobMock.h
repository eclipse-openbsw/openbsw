// Copyright 2024 Accenture.

#pragma once

#include "uds/base/AbstractDiagJob.h"
#include "uds/connection/IncomingDiagConnection.h"

#include <gmock/gmock.h>

namespace uds
{
class DiagJobMock : public AbstractDiagJob
{
public:
    DiagJobMock(
        uint8_t const implementedRequest[],
        uint8_t const requestLength,
        uint8_t const prefixLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS())
    : AbstractDiagJob(implementedRequest, requestLength, prefixLength, sessionMask)
    {}

    DiagJobMock(
        uint8_t const implementedRequest[],
        uint8_t const requestLength,
        uint8_t const prefixLength,
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS())
    : AbstractDiagJob(
        implementedRequest,
        requestLength,
        prefixLength,
        requestPayloadLength,
        responseLength,
        sessionMask)
    {}

    MOCK_METHOD(DiagReturnCode::Type, verify, (uint8_t const[], uint16_t), (override));
    MOCK_METHOD(
        DiagReturnCode::Type,
        process,
        (IncomingDiagConnection&, uint8_t const[], uint16_t),
        (override));
};

#define STATIC_CONCAT(X, Y)  X##Y
#define STATIC_CONCAT2(X, Y) STATIC_CONCAT(X, Y)

#define DIAG_JOB(NAME, REQ, PRE)                   \
    uint8_t STATIC_CONCAT2(req, __LINE__)[] = REQ; \
    DiagJobMock NAME(STATIC_CONCAT2(req, __LINE__), sizeof(STATIC_CONCAT2(req, __LINE__)), PRE);

} // namespace uds
