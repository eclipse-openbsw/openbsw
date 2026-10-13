/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#include "udsDiagMgr/DiagMgr.h"

#include <cstddef>

namespace uds
{

DiagMgr::DiagMgr() : _callbackCount(0U)
{
    for (size_t i = 0U; i < MAX_NOTIFICATION_CALLBACKS; ++i)
    {
        _callbacks[i] = nullptr;
    }
}

void DiagMgr::init(IFaultStorage* storage, ITimeProvider* timeProvider)
{
    _eventManager.init(_faultManager);
    _faultManager.init(storage, timeProvider);
}

void DiagMgr::mainFunction() { _eventManager.mainFunction(MAIN_FUNCTION_INTERVAL_MS); }

// -------------------------------------------------------------------------
// Fault Reporting
// -------------------------------------------------------------------------

void DiagMgr::reportFault(uint32_t eventId, bool isFailed)
{
    _eventManager.setEventStatus(eventId, isFailed);
}

void DiagMgr::clearFault(uint32_t eventId)
{
    // Report pass to debounce the event back to PASSED state
    _eventManager.setEventStatus(eventId, false);
}

uint8_t DiagMgr::getFaultStatus(uint32_t eventId) const
{
    return _faultManager.getDtcStatus(eventId);
}

uint32_t DiagMgr::getFaultCount() const
{
    // Count records with confirmedDTC bit set
    static constexpr uint16_t QUERY_BUFFER_SIZE = 4U; // 1 entry probe
    uint8_t probe[QUERY_BUFFER_SIZE]            = {};
    uint16_t len                                = 0U;
    return static_cast<uint32_t>(
        _faultManager.getDTCList(0x08U /* confirmedDTC */, probe, sizeof(probe), len));
}

bool DiagMgr::registerEvent(EventConfig const& config)
{
    return _eventManager.registerEvent(config);
}

// -------------------------------------------------------------------------
// UDS Query APIs
// -------------------------------------------------------------------------

uint8_t DiagMgr::getDTCList(
    uint8_t statusMask, uint8_t* outBuffer, uint16_t bufferSize, uint16_t& outLength) const
{
    return _faultManager.getDTCList(statusMask, outBuffer, bufferSize, outLength);
}

bool DiagMgr::getFreezeFrame(
    uint32_t dtcId,
    uint8_t recordNumber,
    uint8_t* outBuffer,
    uint16_t bufferSize,
    uint16_t& outLength) const
{
    return _faultManager.getFreezeFrame(dtcId, recordNumber, outBuffer, bufferSize, outLength);
}

bool DiagMgr::getExtendedData(
    uint32_t dtcId,
    uint8_t recordNumber,
    uint8_t* outBuffer,
    uint16_t bufferSize,
    uint16_t& outLength) const
{
    return _faultManager.getExtendedData(dtcId, recordNumber, outBuffer, bufferSize, outLength);
}

bool DiagMgr::clearAllDTCs() { return _faultManager.clearAllDTCs(); }

void DiagMgr::setDTCSettingEnabled(bool enabled) { _faultManager.setDTCSettingEnabled(enabled); }

// -------------------------------------------------------------------------
// Feature Inhibition
// -------------------------------------------------------------------------

bool DiagMgr::isFunctionAllowed(uint16_t functionId) const
{
    return _faultManager.getInhibitionTable().isAllowed(functionId);
}

bool DiagMgr::registerInhibitionEntry(InhibitionEntry const& entry)
{
    return _faultManager.getInhibitionTable().registerEntry(entry);
}

// -------------------------------------------------------------------------
// Suppression
// -------------------------------------------------------------------------

void DiagMgr::setEventSuppression(uint32_t eventId, bool suppress)
{
    _eventManager.setEventSuppression(eventId, suppress);
}

bool DiagMgr::isEventSuppressed(uint32_t eventId) const
{
    return _eventManager.isEventSuppressed(eventId);
}

// -------------------------------------------------------------------------
// Health Telemetry
// -------------------------------------------------------------------------

uint32_t DiagMgr::getWatchdogResetCount() const { return _faultManager.getWatchdogResetCount(); }

uint32_t DiagMgr::getUnexpectedResetCount() const
{
    return _faultManager.getUnexpectedResetCount();
}

uint32_t DiagMgr::getStackOverflowCount() const { return _faultManager.getStackOverflowCount(); }

uint8_t DiagMgr::getStorageFillPercent() const { return _faultManager.getStorageFillPercent(); }

void DiagMgr::incrementWatchdogResetCount() { _faultManager.incrementWatchdogResetCount(); }

void DiagMgr::incrementUnexpectedResetCount() { _faultManager.incrementUnexpectedResetCount(); }

void DiagMgr::incrementStackOverflowCount() { _faultManager.incrementStackOverflowCount(); }

// -------------------------------------------------------------------------
// Fault Notification
// -------------------------------------------------------------------------

bool DiagMgr::registerFaultNotification(FaultNotificationCallback callback)
{
    if (_callbackCount >= MAX_NOTIFICATION_CALLBACKS || callback == nullptr)
    {
        return false;
    }
    _callbacks[_callbackCount] = callback;
    ++_callbackCount;
    return true;
}

void DiagMgr::notifyFaultStatus(uint32_t eventId, uint8_t newStatus)
{
    for (size_t i = 0U; i < _callbackCount; ++i)
    {
        if (_callbacks[i] != nullptr)
        {
            _callbacks[i](eventId, newStatus);
        }
    }
}

// -------------------------------------------------------------------------
// Operation Cycle
// -------------------------------------------------------------------------

void DiagMgr::onOperationCycleStart() { _eventManager.onOperationCycleStart(); }

void DiagMgr::onOperationCycleEnd()
{
    _eventManager.onOperationCycleEnd(); // also calls FaultManager::onOperationCycleEnd()
}

} // namespace uds
