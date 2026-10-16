// Copyright 2026 KPIT Technologies
// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <cstdint>

namespace uds
{

/// Raw event signal states reported by SWCs and produced by debounce algorithms
enum class EventStatus : uint8_t
{
    PREFAILED = 0x00U, ///< Accumulating failure evidence (below threshold)
    PREPASSED = 0x01U, ///< Accumulating pass evidence (below threshold)
    FAILED    = 0x02U, ///< Debounce threshold crossed — failure confirmed
    PASSED    = 0x03U  ///< Debounce threshold crossed — event healed
};

/// Debounce algorithm type selector (DGN-FEL008)
enum class DebounceAlgorithmType : uint8_t
{
    IMMEDIATE  = 0U, ///< No debounce; direct qualification (Algorithm 0)
    COUNTER    = 1U, ///< Counter increment/decrement per call (Algorithm 1)
    TIME_BASED = 2U, ///< Accumulate elapsed ms; confirm after threshold (Algorithm 2)
    HYSTERESIS = 3U  ///< State-machine with separate entry/exit bands (Algorithm 3)
};

/**
 * Pluggable debounce algorithm interface (DGN-FEL002, DGN-FEL008).
 * All concrete algorithms implement this interface.
 */
class IDebounceAlgorithm
{
public:
    virtual ~IDebounceAlgorithm() = default;

    /**
     * Update debounce state.
     * \param isFailed   true = SWC reports failure; false = SWC reports pass
     * \param elapsedMs  Time elapsed since last call (used by time-based algorithm)
     * \return PREFAILED/PREPASSED while accumulating; FAILED/PASSED when threshold crossed
     */
    virtual EventStatus update(bool isFailed, uint16_t elapsedMs) = 0;

    /** Reset debounce counters/accumulators to initial state. */
    virtual void reset() = 0;
};

} // namespace uds
