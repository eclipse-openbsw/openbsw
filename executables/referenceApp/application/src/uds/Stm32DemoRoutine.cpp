// Copyright 2025 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#include "uds/Stm32DemoRoutine.h"

#include "uds/UdsConstants.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/DiagSession.h"

namespace uds
{

Stm32DemoRoutine::Stm32DemoRoutine(uint16_t const routineId)
: RoutineControlJob(_implRequest, sizeof(_implRequest), &_stopNode, &_resultsNode)
, _stopNode(_stopImplRequest, *this)
, _resultsNode(_resultsImplRequest, *this)
{
    // Start routine: [0x31, 0x01, routineId_hi, routineId_lo]
    _implRequest[0] = ServiceId::ROUTINE_CONTROL;
    _implRequest[1] = 0x01U;
    _implRequest[2] = static_cast<uint8_t>((routineId >> 8U) & 0xFFU);
    _implRequest[3] = static_cast<uint8_t>(routineId & 0xFFU);

    // Stop routine: [0x31, 0x02, routineId_hi, routineId_lo]
    _stopImplRequest[0] = ServiceId::ROUTINE_CONTROL;
    _stopImplRequest[1] = 0x02U;
    _stopImplRequest[2] = _implRequest[2];
    _stopImplRequest[3] = _implRequest[3];

    // Request results: [0x31, 0x03, routineId_hi, routineId_lo]
    _resultsImplRequest[0] = ServiceId::ROUTINE_CONTROL;
    _resultsImplRequest[1] = 0x03U;
    _resultsImplRequest[2] = _implRequest[2];
    _resultsImplRequest[3] = _implRequest[3];

    disableSequenceCheck();
}

DiagReturnCode::Type Stm32DemoRoutine::start(
    IncomingDiagConnection& connection,
    uint8_t const* const /* request */,
    uint16_t const /* requestLength */)
{
    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(0x01U);
    (void)response.appendUint8(_implRequest[2]);
    (void)response.appendUint8(_implRequest[3]);
    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

DiagReturnCode::Type Stm32DemoRoutine::stop(
    IncomingDiagConnection& connection,
    uint8_t const* const /* request */,
    uint16_t const /* requestLength */)
{
    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(0x02U);
    (void)response.appendUint8(_implRequest[2]);
    (void)response.appendUint8(_implRequest[3]);
    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

DiagReturnCode::Type Stm32DemoRoutine::requestResults(
    IncomingDiagConnection& connection,
    uint8_t const* const /* request */,
    uint16_t const /* requestLength */)
{
    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(0x03U);
    (void)response.appendUint8(_implRequest[2]);
    (void)response.appendUint8(_implRequest[3]);
    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

} // namespace uds
