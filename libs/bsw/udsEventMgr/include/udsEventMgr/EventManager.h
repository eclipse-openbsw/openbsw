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

#include "udsEventMgr/DebounceCounter.h"
#include "udsEventMgr/DebounceImmediate.h"
#include "udsEventMgr/DebounceTimeBased.h"
#include "udsEventMgr/IDebounceAlgorithm.h"
#include "udsFaultMgr/FaultManagementConfig.h"
#include <cstddef>
#include <cstdint>

namespace uds
{

class FaultManager;

/**
 * Per-event debounce configuration — registered at application init time.
 */
struct EventConfig
{
    uint32_t eventId;                   ///< OEM-specific event/DTC ID
    DebounceAlgorithmType debounceType; ///< Selected debounce algorithm
    uint16_t failThreshold;             ///< Cycles (counter) or ms (time-based) to confirm fail
    uint16_t passThreshold;             ///< Cycles or ms to confirm pass/heal
    uint8_t agingThreshold;             ///< Operation cycles before auto-clear
};

/**
 * EventManager: raw event processing, debounce, and qualification .
 *
 * Receives pass/fail signals from application components via DiagMgr and drives FaultManager
 * when debounce thresholds are crossed.
 *
 * Static allocation only. No heap.
 */
class EventManager
{
public:
    EventManager();

    /** Wire FaultManager — must be called before setEventStatus(). */
    void init(FaultManager& faultManager);

    /**
     * Register an event with its debounce configuration.
     * Must be called before setEventStatus() for a given eventId.
     * \return true if registered successfully; false if table full or duplicate
     */
    bool registerEvent(EventConfig const& config);

    /**
     * Report raw event signal from an application component.
     * Applies the configured debounce algorithm and notifies FaultManager on qualification.
     * \param eventId   Registered event/DTC ID
     * \param isFailed  true = failure condition active; false = condition resolved
     */
    void setEventStatus(uint32_t eventId, bool isFailed);

    /**
     * Periodic main function — drives time-based debounce accumulation.
     * Call every 10 ms from DiagMgr::mainFunction().
     * \param elapsedMs  Milliseconds since last call (typically 10)
     */
    void mainFunction(uint16_t elapsedMs);

    /** Signal start of an operation cycle . */
    void onOperationCycleStart();

    /** Signal end of operation cycle — forwards to FaultManager. */
    void onOperationCycleEnd();

    /** Returns true if the event is currently suppressed . */
    bool isEventSuppressed(uint32_t eventId) const;

    /** Set/clear runtime suppression for an event . */
    void setEventSuppression(uint32_t eventId, bool suppress);

private:
    static constexpr uint8_t MAX_EVENTS = FaultManagementConfig::MAX_EVENTS;

    /**
     * Per-event state. Debounce algorithm objects stored inline (static allocation).
     * activeAlgoType selects which of the three concrete objects is used.
     */
    struct EventEntry
    {
        EventConfig config;
        EventStatus status;
        bool active;
        bool suppressed;
        DebounceAlgorithmType activeAlgoType;
        DebounceImmediate algoImmediate;
        DebounceCounter algoCounter;
        DebounceTimeBased algoTimeBased;
    };

    EventEntry _events[MAX_EVENTS];
    size_t _eventCount;
    FaultManager* _faultManager;

    EventEntry* findEvent(uint32_t eventId);
    EventEntry const* findEvent(uint32_t eventId) const;
    IDebounceAlgorithm& getAlgorithm(EventEntry& entry);
};

} // namespace uds
