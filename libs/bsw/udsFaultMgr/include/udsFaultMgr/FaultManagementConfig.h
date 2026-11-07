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
#include <etl/array.h>
#include <cstdint>

namespace uds
{

/**
 * Compile-time configuration for the FaultManagement subsystem.
 * Edit values per project variant.
 */
struct FaultManagementConfig
{
    // Reduced to lower RAM usage for constrained platforms.
    static constexpr uint8_t MAX_EVENTS = 32U;

    // Reduced default; integrator may increase if needed for their product.
    static constexpr uint8_t MAX_DTC_RECORDS = 8U;

    static constexpr uint8_t DEFAULT_AGING_THRESHOLD = 40U;

    static constexpr bool USE_GLOBAL_TIME = true;

    // Reported in $19 02 response header.
    static constexpr uint8_t DTC_STATUS_AVAILABILITY_MASK = 0xFFU;

    static constexpr etl::array<uint16_t, 3> FREEZE_FRAME_DIDS = {0xCF01U, 0xCF02U, 0xCF03U};
    static constexpr uint8_t FREEZE_FRAME_DID_COUNT            = 3U;

    // Exposed via ReadDataByIdentifier $22.
    static constexpr uint16_t DID_WATCHDOG_RESET_COUNT   = 0xD100U;
    static constexpr uint16_t DID_UNEXPECTED_RESET_COUNT = 0xD101U;
    static constexpr uint16_t DID_STACK_OVERFLOW_COUNT   = 0xD102U;
    static constexpr uint16_t DID_STORAGE_FILL_PERCENT   = 0xD103U;

    static constexpr uint16_t FREEZE_FRAME_SNAPSHOT_SIZE = 32U;
};

} // namespace uds
