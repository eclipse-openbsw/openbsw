/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "uds/jobs/RoutineControlJob.h"

namespace uds
{
/**
 * Demo routine for the STM32 diagnostic demo.
 * Handles start/stop/requestResults for a single routine ID.
 * Returns positive response with no data for all subfunctions.
 */
class Stm32DemoRoutine : public RoutineControlJob
{
public:
    explicit Stm32DemoRoutine(uint16_t routineId);

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
