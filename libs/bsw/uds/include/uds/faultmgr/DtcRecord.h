// Copyright 2026 KPIT Technologies
// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <cstdint>

namespace uds
{

/**
 * ISO 14229-1 Annex B DTC Status Byte bit masks.
 */
struct DtcStatusBits
{
    static constexpr uint8_t TEST_FAILED                         = 0x01U; ///< bit 0
    static constexpr uint8_t TEST_FAILED_THIS_OP_CYCLE           = 0x02U; ///< bit 1
    static constexpr uint8_t PENDING_DTC                         = 0x04U; ///< bit 2
    static constexpr uint8_t CONFIRMED_DTC                       = 0x08U; ///< bit 3
    static constexpr uint8_t TEST_NOT_COMPLETED_SINCE_LAST_CLEAR = 0x10U; ///< bit 4
    static constexpr uint8_t TEST_FAILED_SINCE_LAST_CLEAR        = 0x20U; ///< bit 5
    static constexpr uint8_t TEST_NOT_COMPLETED_THIS_OP_CYCLE    = 0x40U; ///< bit 6
    static constexpr uint8_t WARNING_INDICATOR_REQUESTED         = 0x80U; ///< bit 7
};

/**
 * Full DTC record persisted to NVM per DGN-FEL009.
 * CRC-32 field covers all preceding bytes (DGN-FEL010).
 * Dual nanosecond timestamps for multi-milestone logging (DGN-FEL011).
 */
struct DtcRecord
{
    uint32_t dtcId;                ///< OEM-specific DTC code (DGN-FEL001)
    uint8_t statusByte;            ///< ISO 14229-1 Annex B status byte
    uint8_t failureCounter;        ///< Pre-confirmation detection counter
    uint8_t confirmationCounter;   ///< Post-confirmation occurrence count
    uint8_t occurrenceCounter;     ///< Total occurrences since first detection
    uint8_t agingCounter;          ///< Healing cycles without re-occurrence
    uint8_t agingThreshold;        ///< Max aging cycles before auto-clear
    bool suppressionEnabled;       ///< Runtime suppression flag (DGN-FEL005)
    uint64_t emergingTimestampNs;  ///< Nanosecond timestamp "Emerging" milestone (DGN-FEL011)
    uint64_t confirmedTimestampNs; ///< Nanosecond timestamp "Confirmed" milestone (DGN-FEL011)
    uint32_t crc32;                ///< CRC-32 over all fields above (DGN-FEL010)

    bool isValid() const { return dtcId != 0U; }
};

/// Maximum DID count in one freeze frame snapshot
static constexpr uint8_t FREEZE_FRAME_MAX_DIDS = 8U;
/// Maximum snapshot data bytes per freeze frame
static constexpr uint16_t FREEZE_FRAME_SNAPSHOT_SIZE
    = 32U; // kept in sync with FaultManagementConfig

/**
 * Freeze Frame record — environmental snapshot at DTC confirmation (DGN-FEL003).
 * Timestamped with globally synchronized time (DGN-FEL012).
 * CRC-32 protected (DGN-FEL010).
 */
struct FreezeFrameRecord
{
    uint32_t dtcId;                                   ///< Parent DTC ID
    uint8_t recordNumber;                             ///< Freeze frame record number
    uint64_t timestampNs;                             ///< Global synchronized time (DGN-FEL012)
    uint16_t dataIdentifiers[FREEZE_FRAME_MAX_DIDS];  ///< Captured DID list
    uint8_t snapshotData[FREEZE_FRAME_SNAPSHOT_SIZE]; ///< Raw DID data at confirmation
    uint8_t snapshotLength;                           ///< Actual bytes used in snapshotData
    uint32_t crc32;                                   ///< CRC over all fields above
};

} // namespace uds
