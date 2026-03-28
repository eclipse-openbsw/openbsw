// Copyright 2025 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#pragma once

#include "uds/jobs/RoutineControlJob.h"

namespace uds
{
/**
 * Demo routine for cross-validation with Rust HIL suite.
 * Handles start/stop/requestResults for a single routine ID.
 * Returns positive response with no data for all subfunctions.
 */
class Stm32DemoRoutine : public RoutineControlJob
{
public:
    Stm32DemoRoutine(uint16_t routineId);

    DiagReturnCode::Type start(
        IncomingDiagConnection& connection,
        uint8_t const* request,
        uint16_t requestLength) override;

    DiagReturnCode::Type stop(
        IncomingDiagConnection& connection,
        uint8_t const* request,
        uint16_t requestLength) override;

    DiagReturnCode::Type requestResults(
        IncomingDiagConnection& connection,
        uint8_t const* request,
        uint16_t requestLength) override;

private:
    uint8_t _implRequest[4]; // SID + subFn + routineId[2]
    RoutineControlJobNode _stopNode;
    RoutineControlJobNode _resultsNode;
    uint8_t _stopImplRequest[4];
    uint8_t _resultsImplRequest[4];
};

} // namespace uds
