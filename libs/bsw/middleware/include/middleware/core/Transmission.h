/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <cstdint>

#include "middleware/core/Timer.h"

namespace middleware::core
{

/**
 * \brief This class implements a cyclic transmission.
 *
 * \tparam CyclicTimeMs The cyclic time in milliseconds.
 */
template<uint16_t CyclicTimeMs>
class CyclicTransmission
{
    static_assert(CyclicTimeMs > 0U, "CyclicTimeMs must be greater than zero.");

public:
    CyclicTransmission() { timer_.expire(); }

    /**
     * \brief Counts down the cyclic timer.
     *
     * \return True if the timer was expired and restarted, false otherwise.
     */
    bool execute()
    {
        if (timer_.tick())
        {
            return false;
        }

        timer_.restart();
        return true;
    }

    /**
     * \brief Restarts the cyclic timer from full period.
     *        Used when switching transmission modes.
     */
    void restartTimer() { timer_.restart(); }

private:
    Ticker<uint16_t, CyclicTimeMs> timer_{};
};

/**
 * \brief This class implements event-triggered transmission with optional repetitions.
 *
 * Does not own or transmit the PDU; it signals whether a send can happen upon request.
 *
 * The number of repetitions and the repetition interval must either both be zero or both be greater
 * than zero (enforced via static_assert).
 *
 * \tparam DebounceTimeMs The debounce time in milliseconds.
 * \tparam RepetitionIntervalMs The interval between repetitions in milliseconds. Required.
 * \tparam Repetitions The number of repetitions after the initial event trigger. Default is 2.
 */
template<uint16_t DebounceTimeMs, uint16_t RepetitionIntervalMs, uint8_t Repetitions = 2U>
class EventTransmission
{
    static_assert(
        ((Repetitions == 0U) && (RepetitionIntervalMs == 0U))
            || ((Repetitions > 0U) && (RepetitionIntervalMs > 0U)),
        "Repetitions and RepetitionIntervalMs must either both be zero or both be greater than "
        "zero.");

public:
    EventTransmission() { debounceTimer_.expire(); }

    /**
     * \brief Called every cycle by the owning TransmissionMode to tick the debounce and repetition
     *        timers.
     *
     * Releases a pending event after debounce expires and starts its repetition sequence.
     * Multiple pending events coalesce. Repetitions do not restart debounce.
     *
     * \see TransmissionModeManager::cyclic()
     *
     * \return true if a transmission should happen, false otherwise.
     */
    bool execute()
    {
        debounceTimer_.tick();

        if (pendingEvent_ && debounceTimer_.isExpired())
        {
            pendingEvent_ = false;
            debounceTimer_.restart();
            if constexpr (Repetitions > 0U)
            {
                repetitionTimer_.restart();
                remainingRepetitions_ = Repetitions;
            }
            return true;
        }

        if constexpr ((Repetitions == 0U) || (RepetitionIntervalMs == 0U))
        {
            return false;
        }
        else
        {
            bool const isRepTimerRunning = repetitionTimer_.tick();
            if ((remainingRepetitions_ == 0U) || isRepTimerRunning)
            {
                return false;
            }

            repetitionTimer_.restart();
            --remainingRepetitions_;
            return true;
        }
    }

    /**
     * \brief Invoked when a PDU writer requests sending the PDU (via TransmissionModeManager and
     *        TransmissionMode).
     *
     * Remembers the request even during debounce. Only execute() releases event transmissions.
     *
     * \return false because the request is deferred until execute().
     */
    bool trigger()
    {
        pendingEvent_ = true;
        return false;
    }

    /**
     * \brief Cancels pending events and repetitions when leaving a transmission mode.
     */
    void cancelPendingTransmissions()
    {
        pendingEvent_         = false;
        remainingRepetitions_ = 0U;
    }

private:
    Ticker<uint16_t, DebounceTimeMs> debounceTimer_{};
    Ticker<uint16_t, RepetitionIntervalMs> repetitionTimer_{};

    uint8_t remainingRepetitions_{};
    bool pendingEvent_{};
};

/**
 * \brief This Transmission does nothing.
 *        Meant to be used when no Transmission is to be given.
 */
class NoTransmission
{
public:
    NoTransmission() = default;

    /**
     * \brief Does nothing.
     */
    static bool execute() { return false; }
};

} // namespace middleware::core
