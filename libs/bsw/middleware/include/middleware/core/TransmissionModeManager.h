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

#include "middleware/core/Transmission.h"
#include "middleware/core/TransmissionMode.h"

namespace middleware::core
{

/**
 * \brief Combines two TransmissionMode instances and switches between them at runtime based on a
 *        boolean condition.
 *
 * Does not own or transmit the PDU itself; instead it returns whether the PDU should be sent,
 * leaving the actual transmission to the PDU owner.
 *
 * - Holds two transmission modes and delegates to the currently active one.
 * - Switches the active mode via setActiveCondition(): when the condition is true the second mode
 *   is active, otherwise the first.
 *
 * \tparam TMode1 The first transmission mode.
 * \tparam TMode2 The second transmission mode (defaults to TransmissionMode<NoTransmission>).
 */
template<typename TMode1, typename TMode2 = TransmissionMode<NoTransmission>>
class TransmissionModeManager final
{
    enum class TransmitterState : uint8_t
    {
        Mode1,
        Mode2,
    };

public:
    /**
     * \brief Executes the active mode and consumes its pending cyclic-mode-entry notification.
     * \return True if the active mode or its entry requested a send, false otherwise.
     */
    bool cyclic()
    {
        bool const requested
            = (state_ == TransmitterState::Mode1) ? mode1_.execute() : mode2_.execute();
        bool const send   = requested || pendingModeEntry_;
        pendingModeEntry_ = false;
        return send;
    }

    /**
     * \brief Triggers an event on the active transmission mode.
     *        EventTransmission records the request for a later cyclic() call.
     * \return True if any strategy in the active mode requested a send, false otherwise.
     */
    bool triggerEvent()
    {
        return (state_ == TransmitterState::Mode1) ? mode1_.triggerEvent() : mode2_.triggerEvent();
    }

    /**
     * \brief Sets the active transmission mode based on the given condition.
     *        If true, the second mode is active, else the first mode is active.
     *        Cancels old-mode events and repetitions; cyclic-mode entry is sent on cyclic().
     * \return True if the active mode was changed and a cyclic timer was restarted, false
     *         otherwise.
     */
    bool setActiveCondition(bool const condition)
    {
        TransmitterState const newState
            = condition ? TransmitterState::Mode2 : TransmitterState::Mode1;
        if (newState == state_)
        {
            return false;
        }

        if (state_ == TransmitterState::Mode1)
        {
            cancelPendingTransmissions_(mode1_);
        }
        else
        {
            cancelPendingTransmissions_(mode2_);
        }

        state_ = newState;
        if (newState == TransmitterState::Mode1)
        {
            pendingModeEntry_ = mode1_.restartCyclicTimer();
        }
        else
        {
            pendingModeEntry_ = mode2_.restartCyclicTimer();
        }
        return pendingModeEntry_;
    }

private:
    template<typename TMode>
    static void cancelPendingTransmissions_(TMode& mode)
    {
        if constexpr (HasCancelPendingTransmissions<TMode>::value)
        {
            mode.cancelPendingTransmissions();
        }
    }

    TMode1 mode1_;
    TMode2 mode2_;
    TransmitterState state_{TransmitterState::Mode1};
    bool pendingModeEntry_{};
};

} // namespace middleware::core
