/**
 * @file CanBusOffPolicy.h
 * @brief Host-testable CAN bus-off debounce / gating / recovery decision logic.
 *
 * @details
 * This class implements the bus-off state machine as a pure, hardware-free
 * component that can be unit-tested on any host. It is designed to be driven
 * by a platform adapter (e.g. CanSystem) that receives transceiver state-change
 * callbacks and forwards them here.
 *
 * The state machine enforces three rules:
 *   - A per-controller bus-off occurrence counter is incremented (saturating) on
 *     every bus-off event; the failure is declared only once the counter reaches
 *     tripCount (default: 10).
 *   - Failure declaration and counter-path recovery are only performed while
 *     both application-level gating conditions hold: network mode active AND
 *     no memory corruption detected.
 *   - Recovery (counter path): the controller must report ACTIVE continuously
 *     for recoveryActiveMs (default: 1000 ms) while both gating conditions hold.
 *   - Recovery (unconditional path): resetController()/resetAll() clear the
 *     counter and failure state regardless of gating conditions, modelling
 *     Sleep->WakeUp and MCU Reset recovery.
 *
 * @copyright Copyright 2026 KPIT Technologies.
 *            SPDX-License-Identifier: Apache-2.0
 */

// Copyright 2026 KPIT Technologies.
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <etl/array.h>
#include <platform/estdint.h>

namespace canBusOff
{

/**
 * @class CanBusOffPolicy
 * @brief Pure (no hardware, no I/O) decision logic for CAN bus-off
 *        debounce, gating, and recovery, one instance per CAN channel set.
 */
class CanBusOffPolicy
{
public:
    /// Upper bound on the number of CAN controllers a single policy instance can track.
    static constexpr uint8_t MAX_CONTROLLERS = 16U;

    /// Bus-off is only declared a failure once this many occurrences have been
    /// seen without an intervening recovery.
    static constexpr uint8_t DEFAULT_TRIP_COUNT = 10U;

    /// Recovery (counter path) requires this many milliseconds of continuous
    /// ACTIVE state once the bus-off failure has been recorded.
    static constexpr uint32_t DEFAULT_RECOVERY_ACTIVE_MS = 1000U;

    /**
     * @param numControllers   Number of CAN controllers this instance tracks
     *                         (1..MAX_CONTROLLERS). Any controllerId outside
     *                         [0, numControllers) is treated as invalid and
     *                         ignored by every method below.
     * @param tripCount        Bus-off occurrence count that declares a failure.
     *                         Defaults to DEFAULT_TRIP_COUNT.
     * @param recoveryActiveMs Continuous-ACTIVE duration (ms) required for the
     *                         counter-path recovery. Defaults to DEFAULT_RECOVERY_ACTIVE_MS.
     */
    explicit CanBusOffPolicy(
        uint8_t numControllers,
        uint8_t tripCount         = DEFAULT_TRIP_COUNT,
        uint32_t recoveryActiveMs = DEFAULT_RECOVERY_ACTIVE_MS);

    /**
     * Gating condition: network mode active. When false, failure declaration
     * and counter-path recovery are suppressed. Defaults to true.
     */
    void setNetworkMode(bool underNetworkMode) { _underNetworkMode = underNetworkMode; }
    bool isUnderNetworkMode() const { return _underNetworkMode; }

    /**
     * Gating condition: memory corruption detected. When true, failure
     * declaration and counter-path recovery are suppressed. Defaults to false.
     */
    void setMemoryAreaDataCorruptionDetermined(bool determined)
    {
        _memoryAreaDataCorruptionDetermined = determined;
    }
    bool isMemoryAreaDataCorruptionDetermined() const
    {
        return _memoryAreaDataCorruptionDetermined;
    }

    /**
     * Record one bus-off occurrence for `controllerId`. Increments the
     * saturating counter and, once it reaches tripCount, declares the failure
     * if both gating conditions hold.
     *
     * @return true if this call newly declared the failure (rising edge);
     *         false otherwise (invalid id, not yet tripped, already active,
     *         or gating conditions not met).
     */
    bool onControllerBusOff(uint8_t controllerId);

    /**
     * Report that `controllerId` is in the ACTIVE hardware state at absolute
     * time `nowUs` (free-running, wrap-safe microsecond counter). Starts the
     * continuous-ACTIVE timer on the first call after a bus-off/inactive
     * transition, and triggers counter-path recovery once recoveryActiveMs
     * have elapsed continuously while both gating conditions hold.
     *
     * @return true if this call newly triggered recovery (rising edge);
     *         false otherwise.
     */
    bool onControllerActive(uint8_t controllerId, uint32_t nowUs);

    /**
     * Report that `controllerId` left the ACTIVE state (PASSIVE or BUS_OFF).
     * Cancels any in-progress continuous-ACTIVE timer.
     */
    void onControllerInactive(uint8_t controllerId);

    /**
     * Unconditionally clears the counter and failure state for `controllerId`,
     * regardless of gating conditions. Models the Sleep->WakeUp and MCU Reset
     * recovery paths, which bypass the network-mode and corruption gates.
     *
     * @return true if a previously-active failure was cleared by this call.
     */
    bool resetController(uint8_t controllerId);

    /// Calls resetController() for every tracked controller.
    void resetAll();

    /// Current bus-off occurrence counter for `controllerId` (0 if invalid).
    uint8_t getBusOffCount(uint8_t controllerId) const;

    /// True if the bus-off failure is currently declared for `controllerId`.
    bool isFailureActive(uint8_t controllerId) const;

    /// True if `controllerId` is within the range this instance was constructed for.
    bool isValidController(uint8_t controllerId) const { return controllerId < _numControllers; }

private:
    bool canDeclareOrRecover() const
    {
        return _underNetworkMode && !_memoryAreaDataCorruptionDetermined;
    }

    uint8_t _numControllers;
    uint8_t _tripCount;
    uint32_t _recoveryActiveUs;

    bool _underNetworkMode;
    bool _memoryAreaDataCorruptionDetermined;

    etl::array<uint8_t, MAX_CONTROLLERS> _busOffCount;
    etl::array<bool, MAX_CONTROLLERS> _failureActive;
    etl::array<uint32_t, MAX_CONTROLLERS> _activeSinceUs;
};

} // namespace canBusOff
