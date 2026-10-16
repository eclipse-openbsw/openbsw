// Copyright 2026 KPIT Technologies
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "uds/eventmgr/DebounceCounter.h"
#include "uds/eventmgr/DebounceImmediate.h"
#include "uds/eventmgr/DebounceTimeBased.h"
#include "uds/eventmgr/IDebounceAlgorithm.h"
#include "uds/faultmgr/FaultManagementConfig.h"
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
    DebounceAlgorithmType debounceType; ///< Selected debounce algorithm (DGN-FEL008)
    uint16_t failThreshold;             ///< Cycles (counter) or ms (time-based) to confirm fail
    uint16_t passThreshold;             ///< Cycles or ms to confirm pass/heal
    uint8_t agingThreshold;             ///< Operation cycles before auto-clear (DGN-FEL004)
};

/**
 * EventManager: raw event processing, debounce, and qualification (DGN-FEL002, DGN-FEL008).
 *
 * Receives pass/fail signals from SWCs via DiagMgr and drives FaultManager
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
     * Report raw event signal from a SWC.
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

    /** Signal start of an operation cycle (DGN-FEL004). */
    void onOperationCycleStart();

    /** Signal end of operation cycle — forwards to FaultManager (DGN-FEL004). */
    void onOperationCycleEnd();

    /** Returns true if the event is currently suppressed (DGN-FEL005). */
    bool isEventSuppressed(uint32_t eventId) const;

    /** Set/clear runtime suppression for an event (DGN-FEL005). */
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
    uint8_t _eventCount;
    FaultManager* _faultManager;

    EventEntry* findEvent(uint32_t eventId);
    EventEntry const* findEvent(uint32_t eventId) const;
    IDebounceAlgorithm& getAlgorithm(EventEntry& entry);
};

} // namespace uds
