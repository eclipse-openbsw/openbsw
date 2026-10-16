/********************************************************************************
 * Copyright (c) 2026 Contributors to the Eclipse Foundation
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#include "uds/eventmgr/EventManager.h"
#include "uds/faultmgr/FaultManager.h"

namespace uds
{

EventManager::EventManager() : _eventCount(0U), _faultManager(nullptr)
{
    for (uint8_t i = 0U; i < MAX_EVENTS; ++i)
    {
        _events[i].active         = false;
        _events[i].suppressed     = false;
        _events[i].status         = EventStatus::PREPASSED;
        _events[i].activeAlgoType = DebounceAlgorithmType::IMMEDIATE;
    }
}

void EventManager::init(FaultManager& faultManager) { _faultManager = &faultManager; }

bool EventManager::registerEvent(EventConfig const& config)
{
    if (_eventCount >= MAX_EVENTS)
    {
        return false;
    }
    // Prevent duplicate registration
    if (findEvent(config.eventId) != nullptr)
    {
        return false;
    }

    EventEntry& entry    = _events[_eventCount];
    entry.config         = config;
    entry.status         = EventStatus::PREPASSED;
    entry.active         = true;
    entry.suppressed     = false;
    entry.activeAlgoType = config.debounceType;

    // Initialise correct algorithm object
    switch (config.debounceType)
    {
        case DebounceAlgorithmType::COUNTER:
            entry.algoCounter = DebounceCounter(config.failThreshold, config.passThreshold);
            break;
        case DebounceAlgorithmType::TIME_BASED:
            entry.algoTimeBased = DebounceTimeBased(config.failThreshold, config.passThreshold);
            break;
        case DebounceAlgorithmType::IMMEDIATE:
        case DebounceAlgorithmType::HYSTERESIS:
        default:                                break; // DebounceImmediate needs no configuration
    }
    ++_eventCount;
    return true;
}

void EventManager::setEventStatus(uint32_t eventId, bool isFailed)
{
    EventEntry* entry = findEvent(eventId);
    if (entry == nullptr || !entry->active)
    {
        return;
    }
    if (entry->suppressed)
    {
        return; // DGN-FEL005: suppressed event ignored
    }

    EventStatus const prev = entry->status;
    EventStatus const next = getAlgorithm(*entry).update(isFailed, 0U);
    entry->status          = next;

    if (_faultManager == nullptr)
    {
        return;
    }

    // Notify FaultManager only on state transitions
    if (prev != EventStatus::FAILED && next == EventStatus::FAILED)
    {
        _faultManager->onEventConfirmed(eventId);
    }
    else if (prev != EventStatus::PASSED && next == EventStatus::PASSED)
    {
        _faultManager->onEventPassed(eventId);
    }
    else if (next == EventStatus::PREFAILED || next == EventStatus::PREPASSED)
    {
        _faultManager->onEventPrefailed(eventId, isFailed);
    }
}

void EventManager::mainFunction(uint16_t elapsedMs)
{
    // Drive time-based debounce with elapsed time
    for (uint8_t i = 0U; i < _eventCount; ++i)
    {
        EventEntry& entry = _events[i];
        if (!entry.active || entry.suppressed)
        {
            continue;
        }
        if (entry.activeAlgoType != DebounceAlgorithmType::TIME_BASED)
        {
            continue;
        }

        bool const isInFailDir
            = (entry.status == EventStatus::PREFAILED || entry.status == EventStatus::FAILED);
        EventStatus const prev = entry.status;
        EventStatus const next = entry.algoTimeBased.update(isInFailDir, elapsedMs);
        entry.status           = next;

        if (_faultManager != nullptr)
        {
            if (prev != EventStatus::FAILED && next == EventStatus::FAILED)
            {
                _faultManager->onEventConfirmed(entry.config.eventId);
            }
            else if (prev != EventStatus::PASSED && next == EventStatus::PASSED)
            {
                _faultManager->onEventPassed(entry.config.eventId);
            }
        }
    }
}

void EventManager::onOperationCycleStart()
{
    // Per-cycle flags reset is handled in FaultManager::onOperationCycleEnd
}

void EventManager::onOperationCycleEnd()
{
    if (_faultManager != nullptr)
    {
        _faultManager->onOperationCycleEnd();
    }
}

bool EventManager::isEventSuppressed(uint32_t eventId) const
{
    EventEntry const* entry = findEvent(eventId);
    return (entry != nullptr) && entry->suppressed;
}

void EventManager::setEventSuppression(uint32_t eventId, bool suppress)
{
    EventEntry* entry = findEvent(eventId);
    if (entry != nullptr)
    {
        entry->suppressed = suppress;
        if (suppress)
        {
            getAlgorithm(*entry).reset();
        }
    }
}

EventManager::EventEntry* EventManager::findEvent(uint32_t eventId)
{
    for (uint8_t i = 0U; i < _eventCount; ++i)
    {
        if (_events[i].active && _events[i].config.eventId == eventId)
        {
            return &_events[i];
        }
    }
    return nullptr;
}

EventManager::EventEntry const* EventManager::findEvent(uint32_t eventId) const
{
    for (uint8_t i = 0U; i < _eventCount; ++i)
    {
        if (_events[i].active && _events[i].config.eventId == eventId)
        {
            return &_events[i];
        }
    }
    return nullptr;
}

IDebounceAlgorithm& EventManager::getAlgorithm(EventEntry& entry)
{
    switch (entry.activeAlgoType)
    {
        case DebounceAlgorithmType::COUNTER:    return entry.algoCounter;
        case DebounceAlgorithmType::TIME_BASED: return entry.algoTimeBased;
        case DebounceAlgorithmType::IMMEDIATE:
        case DebounceAlgorithmType::HYSTERESIS:
        default:                                return entry.algoImmediate;
    }
}

} // namespace uds
