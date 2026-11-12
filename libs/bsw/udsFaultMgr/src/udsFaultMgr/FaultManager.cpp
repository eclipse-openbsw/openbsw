/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#include "udsFaultMgr/FaultManager.h"
#include "udsFaultMgr/DtcRecord.h"
#include "udsFaultMgr/FaultManagementConfig.h"
#include "udsFaultMgr/IFaultStorage.h"
#include "udsFaultMgr/ITimeProvider.h"

#include <etl/limits.h>
#include <etl/memory.h>
#include <etl/span.h>
#include <cstddef>
#include <cstdint>

namespace uds
{

namespace
{

// ---------------------------------------------------------------------------
// Portable CRC-32 (bitwise, no lookup table) —
// For production: replace with hardware CRC or table-based implementation.
// ---------------------------------------------------------------------------
constexpr uint32_t CRC32_INIT           = 0xFFFFFFFFUL;
constexpr uint32_t CRC32_POLY_REFLECTED = 0xEDB88320UL;
constexpr uint8_t BITS_PER_BYTE         = 8U;

uint32_t crc32_compute(etl::span<uint8_t const> const& data)
{
    uint32_t crc = CRC32_INIT;
    for (uint8_t const byte : data)
    {
        crc ^= static_cast<uint32_t>(byte);
        for (size_t bitIdx = 0U; bitIdx < BITS_PER_BYTE; ++bitIdx)
        {
            auto const mask = static_cast<uint32_t>(-(static_cast<int32_t>(crc & 1U)));
            crc             = (crc >> 1U) ^ (CRC32_POLY_REFLECTED & mask);
        }
    }
    return ~crc;
}

} // namespace

// ---------------------------------------------------------------------------
FaultManager::FaultManager()
: _dtcRecords()
, _freezeFrames()
, _recordCount(0U)
, _dtcSettingEnabled(true)
, _watchdogResetCount(0U)
, _unexpectedResetCount(0U)
, _stackOverflowCount(0U)
, _storage(nullptr)
, _timeProvider(nullptr)
{
    for (size_t i = 0U; i < MAX_DTC_RECORDS; ++i)
    {
        (void)etl::mem_set(&_dtcRecords[i], 1U, static_cast<uint8_t>(0U));
        (void)etl::mem_set(&_freezeFrames[i], 1U, static_cast<uint8_t>(0U));
    }
}

void FaultManager::init(IFaultStorage* storage, ITimeProvider* timeProvider)
{
    _storage      = storage;
    _timeProvider = timeProvider;
    _inhibitionTable.init(*this);

    if (_storage == nullptr)
    {
        return;
    }

    // Load and CRC-validate all stored DTC records
    for (size_t i = 0U; i < MAX_DTC_RECORDS; ++i)
    {
        DtcRecord candidate{};
        if (!_storage->readDtcRecord(static_cast<uint8_t>(i), candidate))
        {
            continue;
        }
        if (!validateRecordCrc(candidate))
        {
            // Attempt redundant backup slot
            DtcRecord backup{};
            bool const backupOk
                = _storage->readDtcRecord(static_cast<uint8_t>(i + MAX_DTC_RECORDS), backup);
            if (backupOk && validateRecordCrc(backup))
            {
                candidate = backup;
            }
            else
            {
                // Re-initialize to safe defaults — integrity error
                ++_stackOverflowCount; // Repurposed as integrity-error counter in reference impl
                continue;
            }
        }
        if (candidate.isValid() && _recordCount < MAX_DTC_RECORDS)
        {
            _dtcRecords[_recordCount] = candidate;

            // Load corresponding freeze frame
            FreezeFrameRecord freezeFrame{};
            if (_storage->readFreezeFrame(static_cast<uint8_t>(i), freezeFrame)
                && validateFreezeFrameCrc(freezeFrame))
            {
                _freezeFrames[_recordCount] = freezeFrame;
            }
            ++_recordCount;
        }
    }

    // Post-load: mark all events as not-completed-since-last-read (ISO 14229-1)
    for (size_t i = 0U; i < _recordCount; ++i)
    {
        _dtcRecords[i].statusByte |= DtcStatusBits::TEST_NOT_COMPLETED_SINCE_LAST_CLEAR
                                     | DtcStatusBits::TEST_NOT_COMPLETED_THIS_OP_CYCLE;
    }
}

// ---------------------------------------------------------------------------
// EventManager callbacks
// ---------------------------------------------------------------------------

void FaultManager::onEventConfirmed(uint32_t eventId)
{
    if (!_dtcSettingEnabled)
    {
        return; // DTC setting globally disabled via $85
    }

    DtcRecord* rec = findDtcRecord(eventId);
    if (rec == nullptr)
    {
        rec = allocateDtcRecord(eventId);
        if (rec == nullptr)
        {
            return; // DTC table full
        }
        rec->agingThreshold      = FaultManagementConfig::DEFAULT_AGING_THRESHOLD;
        rec->emergingTimestampNs = getNowNs(); //  Emerging milestone
    }

    // Update status byte per ISO 14229-1 Annex B
    rec->statusByte |= DtcStatusBits::TEST_FAILED | DtcStatusBits::TEST_FAILED_THIS_OP_CYCLE
                       | DtcStatusBits::PENDING_DTC | DtcStatusBits::TEST_FAILED_SINCE_LAST_CLEAR;
    rec->statusByte &= static_cast<uint8_t>(~DtcStatusBits::TEST_NOT_COMPLETED_THIS_OP_CYCLE);
    rec->statusByte &= static_cast<uint8_t>(~DtcStatusBits::TEST_NOT_COMPLETED_SINCE_LAST_CLEAR);

    if (rec->failureCounter < etl::numeric_limits<uint8_t>::max())
    {
        ++rec->failureCounter;
    }

    bool const wasConfirmed = (rec->statusByte & DtcStatusBits::CONFIRMED_DTC) != 0U;
    if (!wasConfirmed)
    {
        rec->statusByte |= DtcStatusBits::CONFIRMED_DTC;
        rec->confirmedTimestampNs = getNowNs();
        // Capture freeze frame at first confirmation
        auto const idx            = static_cast<uint8_t>(rec - _dtcRecords.data());
        captureFreezeFrame(eventId, idx);
    }

    if (rec->occurrenceCounter < etl::numeric_limits<uint8_t>::max())
    {
        ++rec->occurrenceCounter;
    }
    rec->agingCounter = 0U; // Reset aging on re-occurrence

    auto const idx = static_cast<uint8_t>(rec - _dtcRecords.data());
    persistRecord(idx);
}

void FaultManager::onEventPassed(uint32_t eventId)
{
    DtcRecord* rec = findDtcRecord(eventId);
    if (rec == nullptr)
    {
        return;
    }
    // Clear testFailed; keep confirmedDTC/pendingDTC — aging heals them over cycles
    rec->statusByte &= static_cast<uint8_t>(~DtcStatusBits::TEST_FAILED);
    rec->statusByte &= static_cast<uint8_t>(~DtcStatusBits::TEST_FAILED_THIS_OP_CYCLE);
    rec->statusByte &= static_cast<uint8_t>(~DtcStatusBits::TEST_NOT_COMPLETED_THIS_OP_CYCLE);

    auto const idx = static_cast<uint8_t>(rec - _dtcRecords.data());
    persistRecord(idx);
}

void FaultManager::onEventPrefailed(uint32_t eventId, bool isFailed)
{
    if (!isFailed)
    {
        return; // Pre-pass state; nothing to record
    }
    DtcRecord* rec = findDtcRecord(eventId);
    if (rec == nullptr)
    {
        rec = allocateDtcRecord(eventId);
        if (rec == nullptr)
        {
            return;
        }
        rec->agingThreshold      = FaultManagementConfig::DEFAULT_AGING_THRESHOLD;
        rec->emergingTimestampNs = getNowNs(); // Emerging milestone on first signal
    }
    // Set pendingDTC on first failure evidence
    rec->statusByte |= DtcStatusBits::PENDING_DTC;
}

// ---------------------------------------------------------------------------
// Operation cycle
// ---------------------------------------------------------------------------

void FaultManager::onOperationCycleEnd()
{
    for (size_t i = 0U; i < _recordCount; ++i)
    {
        DtcRecord& rec = _dtcRecords[i];
        if (!rec.isValid())
        {
            continue;
        }

        bool const failedThisCycle
            = (rec.statusByte & DtcStatusBits::TEST_FAILED_THIS_OP_CYCLE) != 0U;

        if (!failedThisCycle)
        {
            // No re-occurrence → increment aging counter
            if (rec.agingCounter < rec.agingThreshold)
            {
                ++rec.agingCounter;
            }
            rec.statusByte &= static_cast<uint8_t>(~DtcStatusBits::PENDING_DTC);
        }
        else
        {
            // Re-occurred this cycle → reset aging
            rec.agingCounter = 0U;
        }

        // Auto-heal: aging complete
        bool const healed = (rec.agingCounter >= rec.agingThreshold);
        if (healed)
        {
            rec.statusByte &= static_cast<uint8_t>(~DtcStatusBits::CONFIRMED_DTC);
            rec.statusByte &= static_cast<uint8_t>(~DtcStatusBits::PENDING_DTC);
            rec.statusByte &= static_cast<uint8_t>(~DtcStatusBits::TEST_FAILED);
            rec.dtcId = 0U; // Invalidate slot for compaction
        }

        // Reset per-cycle bits for next cycle
        rec.statusByte &= static_cast<uint8_t>(~DtcStatusBits::TEST_FAILED_THIS_OP_CYCLE);
        rec.statusByte |= DtcStatusBits::TEST_NOT_COMPLETED_THIS_OP_CYCLE;

        // Persist healed slots (dtcId=0) too, so storage won't reload them on next init()
        if (rec.isValid() || healed)
        {
            persistRecord(static_cast<uint8_t>(i));
        }
    }

    // Compact: remove healed (dtcId==0) slots
    size_t writeIdx = 0U;
    for (size_t readIdx = 0U; readIdx < _recordCount; ++readIdx)
    {
        if (_dtcRecords[readIdx].isValid())
        {
            if (writeIdx != readIdx)
            {
                _dtcRecords[writeIdx]   = _dtcRecords[readIdx];
                _freezeFrames[writeIdx] = _freezeFrames[readIdx];
            }
            ++writeIdx;
        }
    }
    _recordCount = static_cast<uint8_t>(writeIdx);
}

// ---------------------------------------------------------------------------
// Query APIs
// ---------------------------------------------------------------------------

uint8_t FaultManager::getDTCList(
    uint8_t statusMask, uint8_t* outBuffer, uint16_t bufferSize, uint16_t& outLength) const
{
    static constexpr uint32_t BYTE_MASK = 0xFFU;
    etl::span<uint8_t> const out(outBuffer, bufferSize);

    outLength     = 0U;
    uint8_t count = 0U;
    for (size_t i = 0U; i < _recordCount; ++i)
    {
        DtcRecord const& rec = _dtcRecords[i];
        if (!rec.isValid())
        {
            continue;
        }
        if ((rec.statusByte & statusMask) == 0U)
        {
            continue;
        }
        // 4 bytes per entry: 3-byte DTC + 1-byte status
        static constexpr uint16_t ENTRY_SIZE = 4U;
        if ((outLength + ENTRY_SIZE) > bufferSize)
        {
            break;
        }
        out[outLength + 0U] = static_cast<uint8_t>((rec.dtcId >> (2U * BITS_PER_BYTE)) & BYTE_MASK);
        out[outLength + 1U] = static_cast<uint8_t>((rec.dtcId >> BITS_PER_BYTE) & BYTE_MASK);
        out[outLength + 2U] = static_cast<uint8_t>(rec.dtcId & BYTE_MASK);
        out[outLength + 3U] = rec.statusByte;
        outLength           = static_cast<uint16_t>(outLength + ENTRY_SIZE);
        ++count;
    }
    return count;
}

bool FaultManager::getFreezeFrame(
    uint32_t dtcId,
    uint8_t /*recordNumber*/,
    uint8_t* outBuffer,
    uint16_t bufferSize,
    uint16_t& outLength) const
{
    etl::span<uint8_t> const out(outBuffer, bufferSize);
    for (size_t i = 0U; i < _recordCount; ++i)
    {
        if (_dtcRecords[i].dtcId != dtcId)
        {
            continue;
        }
        FreezeFrameRecord const& freezeFrame = _freezeFrames[i];
        auto const needed = static_cast<uint16_t>(1U + freezeFrame.snapshotLength);
        if (needed > bufferSize)
        {
            return false;
        }
        out[0U] = freezeFrame.recordNumber;
        (void)etl::mem_copy(
            freezeFrame.snapshotData.data(),
            static_cast<size_t>(freezeFrame.snapshotLength),
            &out[1U]);
        outLength = needed;
        return true;
    }
    return false;
}

bool FaultManager::getExtendedData(
    uint32_t dtcId,
    uint8_t /*recordNumber*/,
    uint8_t* outBuffer,
    uint16_t bufferSize,
    uint16_t& outLength) const
{
    static constexpr uint32_t BYTE_MASK = 0xFFU;
    etl::span<uint8_t> const out(outBuffer, bufferSize);
    for (size_t i = 0U; i < _recordCount; ++i)
    {
        DtcRecord const& rec = _dtcRecords[i];
        if (rec.dtcId != dtcId)
        {
            continue;
        }
        // Extended data record 0x01: [recordNum(1) | emergingTs(8) | confirmedTs(8) |
        //                              occurrences(1) | aging(1) | confirmations(1)] = 20 bytes
        static constexpr uint16_t EXT_SIZE = 20U;
        if (bufferSize < EXT_SIZE)
        {
            return false;
        }
        static constexpr uint8_t RECORD_NUMBER          = 0x01U;
        static constexpr int8_t TIMESTAMP_HIGH_BYTE_IDX = 7;
        uint16_t off                                    = 0U;
        out[off]                                        = RECORD_NUMBER; // record number
        ++off;

        // Emerging timestamp big-endian
        for (int8_t byteIdx = TIMESTAMP_HIGH_BYTE_IDX; byteIdx >= 0; --byteIdx)
        {
            out[off] = static_cast<uint8_t>(
                (rec.emergingTimestampNs >> (static_cast<uint8_t>(byteIdx) * BITS_PER_BYTE))
                & BYTE_MASK);
            ++off;
        }
        // Confirmed timestamp big-endian
        for (int8_t byteIdx = TIMESTAMP_HIGH_BYTE_IDX; byteIdx >= 0; --byteIdx)
        {
            out[off] = static_cast<uint8_t>(
                (rec.confirmedTimestampNs >> (static_cast<uint8_t>(byteIdx) * BITS_PER_BYTE))
                & BYTE_MASK);
            ++off;
        }
        out[off] = rec.occurrenceCounter;
        ++off;
        out[off] = rec.agingCounter;
        ++off;
        out[off] = rec.confirmationCounter;
        ++off;
        outLength = off;
        return true;
    }
    return false;
}

uint8_t FaultManager::getDtcStatus(uint32_t dtcId) const
{
    DtcRecord const* rec = findDtcRecord(dtcId);
    return (rec != nullptr) ? rec->statusByte : 0U;
}

bool FaultManager::clearAllDTCs()
{
    for (size_t i = 0U; i < _recordCount; ++i)
    {
        (void)etl::mem_set(&_dtcRecords[i], 1U, static_cast<uint8_t>(0U));
        (void)etl::mem_set(&_freezeFrames[i], 1U, static_cast<uint8_t>(0U));
    }
    _recordCount = 0U;

    if (_storage != nullptr)
    {
        return _storage->eraseAll();
    }
    return true;
}

void FaultManager::setDTCSettingEnabled(bool enabled) { _dtcSettingEnabled = enabled; }

uint8_t FaultManager::getStorageFillPercent() const
{
    static constexpr uint32_t PERCENT_SCALE = 100U;
    if (MAX_DTC_RECORDS == 0U)
    {
        return 0U;
    }
    return static_cast<uint8_t>(
        (static_cast<uint32_t>(_recordCount) * PERCENT_SCALE)
        / static_cast<uint32_t>(MAX_DTC_RECORDS));
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

DtcRecord* FaultManager::findDtcRecord(uint32_t dtcId)
{
    for (size_t i = 0U; i < _recordCount; ++i)
    {
        if (_dtcRecords[i].dtcId == dtcId)
        {
            return &_dtcRecords[i];
        }
    }
    return nullptr;
}

DtcRecord const* FaultManager::findDtcRecord(uint32_t dtcId) const
{
    for (size_t i = 0U; i < _recordCount; ++i)
    {
        if (_dtcRecords[i].dtcId == dtcId)
        {
            return &_dtcRecords[i];
        }
    }
    return nullptr;
}

DtcRecord* FaultManager::allocateDtcRecord(uint32_t dtcId)
{
    if (_recordCount >= MAX_DTC_RECORDS)
    {
        return nullptr;
    }
    DtcRecord& rec = _dtcRecords[_recordCount];
    (void)etl::mem_set(&rec, 1U, static_cast<uint8_t>(0U));
    rec.dtcId      = dtcId;
    rec.statusByte = DtcStatusBits::TEST_NOT_COMPLETED_SINCE_LAST_CLEAR
                     | DtcStatusBits::TEST_NOT_COMPLETED_THIS_OP_CYCLE;
    ++_recordCount;
    return &rec;
}

void FaultManager::captureFreezeFrame(uint32_t dtcId, uint8_t index)
{
    // build freeze frame at DTC confirmation
    FreezeFrameRecord& freezeFrame = _freezeFrames[index];
    (void)etl::mem_set(&freezeFrame, 1U, static_cast<uint8_t>(0U));
    freezeFrame.dtcId        = dtcId;
    freezeFrame.recordNumber = 0x01U;
    freezeFrame.timestampNs  = getNowNs(); // global synchronized timestamp

    uint8_t const didCount = FaultManagementConfig::FREEZE_FRAME_DID_COUNT;
    uint8_t const maxDids  = (didCount < FREEZE_FRAME_MAX_DIDS) ? didCount : FREEZE_FRAME_MAX_DIDS;
    uint8_t off            = 0U;
    for (size_t didIdx = 0U; didIdx < maxDids && (off + 1U) < FREEZE_FRAME_SNAPSHOT_SIZE; ++didIdx)
    {
        freezeFrame.dataIdentifiers[didIdx] = FaultManagementConfig::FREEZE_FRAME_DIDS[didIdx];
        // Placeholder: store DID bytes. Project integrator replaces with live DID reads.
        freezeFrame.snapshotData[off]
            = static_cast<uint8_t>(freezeFrame.dataIdentifiers[didIdx] >> BITS_PER_BYTE);
        ++off;
        static constexpr uint32_t BYTE_MASK = 0xFFU;
        freezeFrame.snapshotData[off]
            = static_cast<uint8_t>(freezeFrame.dataIdentifiers[didIdx] & BYTE_MASK);
        ++off;
    }
    freezeFrame.snapshotLength = off;
    updateFreezeFrameCrc(freezeFrame);

    if (_storage != nullptr)
    {
        (void)_storage->writeFreezeFrame(index, freezeFrame);
    }
}

void FaultManager::persistRecord(uint8_t index)
{
    if (_storage == nullptr || index >= _recordCount)
    {
        return;
    }
    updateRecordCrc(_dtcRecords[index]);
    // Primary slot
    (void)_storage->writeDtcRecord(index, _dtcRecords[index]);
    // Redundant backup slot
    (void)_storage->writeDtcRecord(
        static_cast<uint8_t>(index + MAX_DTC_RECORDS), _dtcRecords[index]);
}

uint32_t FaultManager::computeCrc32(etl::span<uint8_t const> const& data)
{
    return crc32_compute(data);
}

bool FaultManager::validateRecordCrc(DtcRecord const& record)
{
    static constexpr auto CRC_PAYLOAD_SIZE = static_cast<uint16_t>(offsetof(DtcRecord, crc32));
    auto const computed                    = computeCrc32(etl::span<uint8_t const>(
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<uint8_t const*>(&record),
        CRC_PAYLOAD_SIZE));
    return computed == record.crc32;
}

bool FaultManager::validateFreezeFrameCrc(FreezeFrameRecord const& record)
{
    static constexpr auto CRC_PAYLOAD_SIZE
        = static_cast<uint16_t>(offsetof(FreezeFrameRecord, crc32));
    auto const computed = computeCrc32(etl::span<uint8_t const>(
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<uint8_t const*>(&record),
        CRC_PAYLOAD_SIZE));
    return computed == record.crc32;
}

void FaultManager::updateRecordCrc(DtcRecord& record)
{
    static constexpr auto CRC_PAYLOAD_SIZE = static_cast<uint16_t>(offsetof(DtcRecord, crc32));
    record.crc32                           = computeCrc32(etl::span<uint8_t const>(
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<uint8_t const*>(&record),
        CRC_PAYLOAD_SIZE));
}

void FaultManager::updateFreezeFrameCrc(FreezeFrameRecord& record)
{
    static constexpr auto CRC_PAYLOAD_SIZE
        = static_cast<uint16_t>(offsetof(FreezeFrameRecord, crc32));
    record.crc32 = computeCrc32(etl::span<uint8_t const>(
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<uint8_t const*>(&record),
        CRC_PAYLOAD_SIZE));
}

uint64_t FaultManager::getNowNs() const
{
    if (_timeProvider != nullptr && _timeProvider->isSynchronized())
    {
        return _timeProvider->getNanoseconds();
    }
    return ITimeProvider::INVALID_TIMESTAMP;
}

} // namespace uds
