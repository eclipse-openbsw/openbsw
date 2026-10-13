// Copyright 2026 KPIT Technologies Ltd
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "uds/eventmgr/EventManager.h"
#include "uds/faultmgr/FaultManagementConfig.h"
#include "uds/faultmgr/FaultManager.h"
#include "uds/faultmgr/IFaultStorage.h"
#include "uds/faultmgr/ITimeProvider.h"
#include <cstdint>

namespace uds
{

/**
 * DiagMgr — Diagnostic Manager Façade (DGN-FEL001 through DGN-FEL012).
 *
 * Single entry point used by:
 *   - Application SWCs (reportFault, isFunctionAllowed)
 *   - UDS services $14, $19, $85 (clear, read, control)
 *   - ReadDataByIdentifier $22 health telemetry DIDs
 *   - Cloud / service tool diagnostic backends
 *
 * Owns EventManager and FaultManager as value members (static allocation).
 * No heap usage. Call init() during UdsSystem::init() before any other method.
 *
 * Architecture reference: ArchitectureReferences.md §DiagMgr
 */
class DiagMgr
{
public:
    // -------------------------------------------------------------------------
    // Fault notification callback type
    // -------------------------------------------------------------------------
    using FaultNotificationCallback = void (*)(uint32_t eventId, uint8_t newStatus);

    static constexpr uint8_t MAX_NOTIFICATION_CALLBACKS = 4U;

    // -------------------------------------------------------------------------
    // Lifecycle
    // -------------------------------------------------------------------------
    DiagMgr();

    /**
     * Initialise DiagMgr and its sub-components.
     * \param storage     NVM storage backend (may be nullptr for RAM-only builds)
     * \param timeProvider  StbM-backed time source (may be nullptr if DGN-FEL012 not needed)
     */
    void init(IFaultStorage* storage = nullptr, ITimeProvider* timeProvider = nullptr);

    /**
     * Periodic main function. Call every 10 ms from UdsSystem::execute().
     * Drives time-based debounce algorithms and health telemetry bookkeeping.
     */
    void mainFunction();

    // -------------------------------------------------------------------------
    // Fault Reporting API — used by SWCs (DGN-FEL001/002)
    // -------------------------------------------------------------------------

    /**
     * Report a raw fault/pass signal for a registered event.
     * \param eventId   OEM-specific event/DTC ID (DGN-FEL001)
     * \param isFailed  true = failure condition detected; false = condition resolved
     */
    void reportFault(uint32_t eventId, bool isFailed);

    /**
     * Directly clear fault state for a single event (e.g. SW-commanded clear).
     */
    void clearFault(uint32_t eventId);

    /**
     * Get the ISO 14229-1 Annex B DTC status byte for a given event/DTC ID.
     * Returns 0x00 if the event has no active DTC record.
     */
    uint8_t getFaultStatus(uint32_t eventId) const;

    /**
     * Get the total number of confirmed DTCs currently stored.
     */
    uint32_t getFaultCount() const;

    // -------------------------------------------------------------------------
    // Event Registration (must be called before reportFault for an eventId)
    // -------------------------------------------------------------------------

    /**
     * Register an event with its debounce configuration.
     * Typically called during application init before UdsSystem::run().
     */
    bool registerEvent(EventConfig const& config);

    // -------------------------------------------------------------------------
    // UDS Query APIs — used by $14, $19, $85 services
    // -------------------------------------------------------------------------

    /**
     * Fill \p outBuffer with all DTCs matching \p statusMask.
     * Format per entry: [DTC_Hi, DTC_Mid, DTC_Lo, StatusByte] (4 bytes).
     * \param statusMask  ISO 14229-1 DTC status mask (e.g. 0x09 = confirmed+testFailed)
     * \param outBuffer   Caller-provided buffer to receive entries
     * \param bufferSize  Size of \p outBuffer in bytes
     * \param outLength   Set to the number of bytes written on return
     * \return Number of DTC entries written
     */
    uint8_t getDTCList(
        uint8_t statusMask, uint8_t* outBuffer, uint16_t bufferSize, uint16_t& outLength) const;

    /**
     * Fill \p outBuffer with freeze frame data for the given DTC (DGN-FEL003).
     * \return true if data was found and copied; false otherwise
     */
    bool getFreezeFrame(
        uint32_t dtcId,
        uint8_t recordNumber,
        uint8_t* outBuffer,
        uint16_t bufferSize,
        uint16_t& outLength) const;

    /**
     * Fill \p outBuffer with extended data for the given DTC (DGN-FEL011).
     * Contains dual timestamps + counters.
     * \return true if data was found and copied; false otherwise
     */
    bool getExtendedData(
        uint32_t dtcId,
        uint8_t recordNumber,
        uint8_t* outBuffer,
        uint16_t bufferSize,
        uint16_t& outLength) const;

    /**
     * Clear all DTC records from RAM and NVM (DGN-FEL009).
     * Called by ClearDiagnosticInformation ($14).
     * \return true on success
     */
    bool clearAllDTCs();

    /**
     * Enable or disable DTC setting ($85 ControlDTCSetting, DGN-FEL005).
     * When disabled, no new DTCs are confirmed.
     */
    void setDTCSettingEnabled(bool enabled);

    // -------------------------------------------------------------------------
    // Feature Inhibition Interface (DGN-FEL006)
    // -------------------------------------------------------------------------

    /**
     * Query whether a software function is permitted to execute.
     * Returns false if any DTC mapped to \p functionId in the InhibitionTable
     * is in confirmedDTC state.
     * Returns true by default for unregistered function IDs.
     */
    bool isFunctionAllowed(uint16_t functionId) const;

    /**
     * Register a function-to-DTC inhibition mapping.
     * \return true if registered; false if table full
     */
    bool registerInhibitionEntry(InhibitionEntry const& entry);

    // -------------------------------------------------------------------------
    // Suppression Mask (DGN-FEL005)
    // -------------------------------------------------------------------------

    /**
     * Set suppression for a specific event at runtime.
     * Suppression mask persisted by project integrator via IFaultStorage extension.
     */
    void setEventSuppression(uint32_t eventId, bool suppress);

    /**
     * Check whether an event is currently suppressed.
     */
    bool isEventSuppressed(uint32_t eventId) const;

    // -------------------------------------------------------------------------
    // System Health Telemetry (DGN-FEL007)
    // -------------------------------------------------------------------------

    /** @{ Telemetry counter accessors for $22 DID read-back */
    uint32_t getWatchdogResetCount() const;
    uint32_t getUnexpectedResetCount() const;
    uint32_t getStackOverflowCount() const;
    uint8_t getStorageFillPercent() const;
    /** @} */

    /** Increment telemetry counters — called by safety/watchdog handlers */
    void incrementWatchdogResetCount();
    void incrementUnexpectedResetCount();
    void incrementStackOverflowCount();

    // -------------------------------------------------------------------------
    // Fault Notification (DGN-FEL006 observer pattern)
    // -------------------------------------------------------------------------

    /**
     * Register a callback to be invoked when a DTC status changes.
     * Up to MAX_NOTIFICATION_CALLBACKS callbacks supported.
     */
    bool registerFaultNotification(FaultNotificationCallback callback);

    /**
     * Notify all registered callbacks of a DTC status change.
     * Called internally by FaultManager (via DiagMgr) after any status update.
     */
    void notifyFaultStatus(uint32_t eventId, uint8_t newStatus);

    // -------------------------------------------------------------------------
    // Operation Cycle Control (DGN-FEL004)
    // -------------------------------------------------------------------------

    /**
     * Signal start of an operation cycle (e.g. ignition-on).
     */
    void onOperationCycleStart();

    /**
     * Signal end of an operation cycle (e.g. ignition-off).
     * Drives aging/healing logic inside FaultManager.
     */
    void onOperationCycleEnd();

    // -------------------------------------------------------------------------
    // Sub-component accessors (for UDS service wiring in UdsSystem)
    // -------------------------------------------------------------------------
    EventManager& getEventManager() { return _eventManager; }

    FaultManager& getFaultManager() { return _faultManager; }

private:
    static constexpr uint16_t MAIN_FUNCTION_INTERVAL_MS = 10U;

    EventManager _eventManager;
    FaultManager _faultManager;

    FaultNotificationCallback _callbacks[MAX_NOTIFICATION_CALLBACKS];
    uint8_t _callbackCount;
};

} // namespace uds
