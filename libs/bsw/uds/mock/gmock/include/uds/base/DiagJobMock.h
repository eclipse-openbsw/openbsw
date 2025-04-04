// Copyright 2024 Accenture.

#pragma once

#include "uds/base/AbstractDiagJob.h"
#include "uds/connection/IncomingDiagConnection.h"

#include <estd/static_assert.h>

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

    MOCK_METHOD3(
        verify, DiagReturnCode::Type(IncomingDiagConnection const&, uint8_t const[], uint16_t));
    MOCK_METHOD2(verify, DiagReturnCode::Type(uint8_t const[], uint16_t));
    MOCK_METHOD3(process, DiagReturnCode::Type(IncomingDiagConnection&, uint8_t const[], uint16_t));
};

#define DIAG_JOB(NAME, REQ, PRE)                        \
    uint8_t ESTD_STATIC_CONCAT2(req, __LINE__)[] = REQ; \
    DiagJobMock NAME(                                   \
        ESTD_STATIC_CONCAT2(req, __LINE__), sizeof(ESTD_STATIC_CONCAT2(req, __LINE__)), PRE);

} // namespace uds
