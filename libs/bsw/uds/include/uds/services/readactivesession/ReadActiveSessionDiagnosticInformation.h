/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#pragma once

#include "uds/jobs/DataIdentifierJob.h"
#include "uds/session/IDiagSessionManager.h"

namespace uds
{

/**
 * @file    ReadActiveSessionDiagnosticInformation.h
 * @brief   UDS service ReadDataByIdentifier ($22) — DID 0xF186
 *          ActiveDiagnosticSessionDataIdentifier
 *
 * ISO 14229-1:2020 §C.1 — Standardized Data Identifier 0xF186
 *
 * This service responds with the currently active UDS diagnostic session byte:
 *   0x01  — DefaultSession
 *   0x02  — ProgrammingSession
 *   0x03  — ExtendedDiagnosticSession
 *
 * ## Request PDU
 * | Byte | Value  | Description                       |
 * |------|--------|-----------------------------------|
 * | 0    | 0x22   | Service ID — ReadDataByIdentifier |
 * | 1    | 0xF1   | DID high byte                     |
 * | 2    | 0x86   | DID low byte                      |
 *
 * ## Positive Response PDU
 * | Byte | Value  | Description                       |
 * |------|--------|-----------------------------------|
 * | 0    | 0x62   | Positive response SID (0x22+0x40) |
 * | 1    | 0xF1   | DID high byte echo                |
 * | 2    | 0x86   | DID low byte echo                 |
 * | 3    | 0xXX   | Active session byte (01/02/03)    |
 *
 * ## Negative Responses
 * | NRC  | Condition                                          |
 * |------|----------------------------------------------------|
 * | 0x22 | conditionsNotCorrect — session manager not wired   |
 * | 0x31 | requestOutOfRange — DID != 0xF186 (parent default) |
 *
 * ## Session Access
 * Accessible in ALL diagnostic sessions. No security access required.
 *
 * ## Stack Placement
 * Extends DataIdentifierJob — a child leaf under ReadDataByIdentifier ($22).
 * Registered via UdsSystem::addDiagJobs() → _jobRoot.addAbstractDiagJob(_read22F186).
 * Wired via UdsSystem::init() → _read22F186.setSessionManager(_diagnosticSessionControl).
 *
 * @see ISO 14229-1:2020 Table C.1 — standardizedDataIdentifier 0xF186
 * @see DataIdentifierJob
 * @see IDiagSessionManager
 */
class ReadActiveSessionDiagnosticInformation : public DataIdentifierJob
{
public:
    /**
     * Constructor.
     * Registers as handler for the 3-byte request [0x22, 0xF1, 0x86].
     * Session mask: ALL_SESSIONS — accessible in any session without security access.
     */
    ReadActiveSessionDiagnosticInformation();

    /**
     * Wire the session manager used to retrieve the active session byte.
     * Must be called before the job is added to the DiagJobRoot.
     * In production: call during UdsSystem::init().
     *
     * @param sessionManager  Reference to the active IDiagSessionManager instance.
     */
    void setSessionManager(IDiagSessionManager const& sessionManager);

private:
    /// DID high byte
    static uint8_t const DID_HIGH = 0xF1U;
    /// DID low byte
    static uint8_t const DID_LOW  = 0x86U;

    /**
     * Process the 0xF186 request.
     * Reads active session from IDiagSessionManager and sends positive response.
     *
     * @param connection    Active incoming diagnostic connection
     * @param request       Request bytes as seen by this job (after SID prefix)
     * @param requestLength Length of request
     * @return DiagReturnCode::OK on success; NRC code on failure
     */
    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    /// Statically allocated 3-byte request [SID=0x22, DID_H=0xF1, DID_L=0x86]
    uint8_t _implementedRequest[3];

    /// Pointer to IDiagSessionManager — injected via setSessionManager()
    IDiagSessionManager const* _sessionManager;
};

} // namespace uds
