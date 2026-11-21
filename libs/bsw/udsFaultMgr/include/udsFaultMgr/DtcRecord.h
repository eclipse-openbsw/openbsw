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
 * ISO 14229-1 Annex B DTC Status Byte bit masks.
 */
struct DtcStatusBits
{
    static constexpr uint8_t TEST_FAILED                         = 0x01U;
    static constexpr uint8_t TEST_FAILED_THIS_OP_CYCLE           = 0x02U;
    static constexpr uint8_t PENDING_DTC                         = 0x04U;
    static constexpr uint8_t CONFIRMED_DTC                       = 0x08U;
    static constexpr uint8_t TEST_NOT_COMPLETED_SINCE_LAST_CLEAR = 0x10U;
    static constexpr uint8_t TEST_FAILED_SINCE_LAST_CLEAR        = 0x20U;
    static constexpr uint8_t TEST_NOT_COMPLETED_THIS_OP_CYCLE    = 0x40U;
    static constexpr uint8_t WARNING_INDICATOR_REQUESTED         = 0x80U;
};

/**
 * Full DTC record persisted to storage per .
 * CRC-32 field covers all preceding bytes .
 * Dual nanosecond timestamps for multi-milestone logging .
 */
struct DtcRecord
{
    uint32_t dtcId;                ///< OEM-specific DTC code
    uint8_t statusByte;            ///< ISO 14229-1 Annex B status byte
    uint8_t failureCounter;        ///< Pre-confirmation detection counter
    uint8_t confirmationCounter;   ///< Post-confirmation occurrence count
    uint8_t occurrenceCounter;     ///< Total occurrences since first detection
    uint8_t agingCounter;          ///< Healing cycles without re-occurrence
    uint8_t agingThreshold;        ///< Max aging cycles before auto-clear
    bool suppressionEnabled;       ///< Runtime suppression flag
    uint64_t emergingTimestampNs;  ///< Nanosecond timestamp "Emerging" milestone
    uint64_t confirmedTimestampNs; ///< Nanosecond timestamp "Confirmed" milestone
    uint32_t crc32;                ///< CRC-32 over all fields above

    bool isValid() const { return dtcId != 0U; }
};

static constexpr uint8_t FREEZE_FRAME_MAX_DIDS       = 8U;
// Kept in sync with FaultManagementConfig::FREEZE_FRAME_SNAPSHOT_SIZE.
static constexpr uint16_t FREEZE_FRAME_SNAPSHOT_SIZE = 32U;

/**
 * Freeze Frame record — environmental snapshot at DTC confirmation
 * Timestamped with globally synchronized time
 * CRC-32 protected
 */
struct FreezeFrameRecord
{
    uint32_t dtcId;                                               ///< Parent DTC ID
    uint8_t recordNumber;                                         ///< Freeze frame record number
    uint64_t timestampNs;                                         ///< Global synchronized time
    etl::array<uint16_t, FREEZE_FRAME_MAX_DIDS> dataIdentifiers;  ///< Captured DID list
    etl::array<uint8_t, FREEZE_FRAME_SNAPSHOT_SIZE> snapshotData; ///< Raw DID data at confirmation
    uint8_t snapshotLength; ///< Actual bytes used in snapshotData
    uint32_t crc32;         ///< CRC over all fields above
};

} // namespace uds
