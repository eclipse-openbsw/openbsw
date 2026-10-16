// Copyright 2026 KPIT Technologies
// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "uds/faultmgr/DtcRecord.h"
#include "uds/faultmgr/FaultManagementConfig.h"
#include "uds/faultmgr/IFaultStorage.h"
#include "uds/faultmgr/ITimeProvider.h"
#include "uds/faultmgr/InhibitionTable.h"
#include <cstdint>

namespace uds
{

/**
 * FaultManager: DTC lifecycle, freeze frames, NVM persistence, and
 * feature inhibition (DGN-FEL001 – DGN-FEL012).
 *
 * Responsibilities:
 *   - Receive qualified events from EventManager; update DTC status bytes.
 *   - Capture freeze frames and extended data at first confirmation (DGN-FEL003).
 *   - Manage aging/healing over operation cycles (DGN-FEL004).
 *   - Persist CRC-protected records to NVM with redundant backup (DGN-FEL009/010).
 *   - Expose query APIs consumed by DiagMgr for UDS $14/$19/$85.
 *   - Track system health telemetry counters (DGN-FEL007).
 *
 * Static allocation only. No heap.
 */
class FaultManager
{
public:
    FaultManager();

    /**
     * Initialize FaultManager. Loads NVM records and validates CRC (DGN-FEL010).
     * \param storage       NVM backend (nullptr for RAM-only builds)
     * \param timeProvider  Global time source (nullptr if DGN-FEL012 not required)
     */
    void init(IFaultStorage* storage, ITimeProvider* timeProvider);

    // -------------------------------------------------------------------------
    // EventManager callbacks
    // -------------------------------------------------------------------------

    /** Called when debounce algorithm transitions event → FAILED (confirmed). */
    void onEventConfirmed(uint32_t eventId);

    /** Called when debounce algorithm transitions event → PASSED (healed). */
    void onEventPassed(uint32_t eventId);

    /** Called during accumulation phase (PREFAILED/PREPASSED) — sets PENDING_DTC. */
    void onEventPrefailed(uint32_t eventId, bool isFailed);

    // -------------------------------------------------------------------------
    // Operation cycle management (DGN-FEL004)
    // -------------------------------------------------------------------------
    void onOperationCycleEnd();

    // -------------------------------------------------------------------------
    // UDS Query APIs (consumed by DiagMgr → $14, $19, $85)
    // -------------------------------------------------------------------------

    /**
     * Fill buffer with [DTC_Hi, DTC_Mid, DTC_Lo, StatusByte] for each matching DTC.
     * \return Number of DTC entries written.
     */
    uint8_t getDTCList(
        uint8_t statusMask, uint8_t* outBuffer, uint16_t bufferSize, uint16_t& outLength) const;

    /** Fill buffer with freeze frame data for the given DTC (DGN-FEL003). */
    bool getFreezeFrame(
        uint32_t dtcId,
        uint8_t recordNumber,
        uint8_t* outBuffer,
        uint16_t bufferSize,
        uint16_t& outLength) const;

    /** Fill buffer with extended data record (timestamps + counters) (DGN-FEL011). */
    bool getExtendedData(
        uint32_t dtcId,
        uint8_t recordNumber,
        uint8_t* outBuffer,
        uint16_t bufferSize,
        uint16_t& outLength) const;

    /** Returns the ISO 14229-1 DTC status byte; 0 if DTC not found. */
    uint8_t getDtcStatus(uint32_t dtcId) const;

    // -------------------------------------------------------------------------
    // Clear — called by DiagMgr for UDS $14
    // -------------------------------------------------------------------------
    bool clearAllDTCs();

    // -------------------------------------------------------------------------
    // DTC Setting control — UDS $85 (DGN-FEL005)
    // -------------------------------------------------------------------------
    void setDTCSettingEnabled(bool enabled);

    bool isDTCSettingEnabled() const { return _dtcSettingEnabled; }

    // -------------------------------------------------------------------------
    // Health telemetry (DGN-FEL007)
    // -------------------------------------------------------------------------
    uint32_t getWatchdogResetCount() const { return _watchdogResetCount; }

    uint32_t getUnexpectedResetCount() const { return _unexpectedResetCount; }

    uint32_t getStackOverflowCount() const { return _stackOverflowCount; }

    uint8_t getStorageFillPercent() const;

    void incrementWatchdogResetCount() { ++_watchdogResetCount; }

    void incrementUnexpectedResetCount() { ++_unexpectedResetCount; }

    void incrementStackOverflowCount() { ++_stackOverflowCount; }

    // -------------------------------------------------------------------------
    // InhibitionTable access
    // -------------------------------------------------------------------------
    InhibitionTable& getInhibitionTable() { return _inhibitionTable; }

    InhibitionTable const& getInhibitionTable() const { return _inhibitionTable; }

private:
    static constexpr uint8_t MAX_DTC_RECORDS = FaultManagementConfig::MAX_DTC_RECORDS;

    DtcRecord _dtcRecords[MAX_DTC_RECORDS];
    FreezeFrameRecord _freezeFrames[MAX_DTC_RECORDS];
    uint8_t _recordCount;
    bool _dtcSettingEnabled;

    uint32_t _watchdogResetCount;
    uint32_t _unexpectedResetCount;
    uint32_t _stackOverflowCount;

    IFaultStorage* _storage;
    ITimeProvider* _timeProvider;
    InhibitionTable _inhibitionTable;

    // Internal helpers
    DtcRecord* findDtcRecord(uint32_t dtcId);
    DtcRecord const* findDtcRecord(uint32_t dtcId) const;
    DtcRecord* allocateDtcRecord(uint32_t dtcId);

    void captureFreezeFrame(uint32_t dtcId, uint8_t index);
    void persistRecord(uint8_t index);

    uint32_t computeCrc32(uint8_t const* data, uint16_t length) const;
    bool validateRecordCrc(DtcRecord const& record) const;
    bool validateFreezeFrameCrc(FreezeFrameRecord const& record) const;
    void updateRecordCrc(DtcRecord& record);
    void updateFreezeFrameCrc(FreezeFrameRecord& record);
    uint64_t getNowNs() const;
};

} // namespace uds
