/**
 * @file CanBusOffPolicy.cpp
 * @brief See CanBusOffPolicy.h for the state machine rules this implements.
 *
 * @copyright Copyright 2026 KPIT Technologies.
 *            SPDX-License-Identifier: Apache-2.0
 */

// Copyright 2026 KPIT Technologies.
// SPDX-License-Identifier: Apache-2.0
#include "canBusOff/CanBusOffPolicy.h"

namespace canBusOff
{

CanBusOffPolicy::CanBusOffPolicy(
    uint8_t const numControllers, uint8_t const tripCount, uint32_t const recoveryActiveMs)
    : _numControllers((numControllers > MAX_CONTROLLERS) ? MAX_CONTROLLERS : numControllers)
    , _tripCount(tripCount)
    , _recoveryActiveUs(recoveryActiveMs * 1000U)
    , _underNetworkMode(true)
    , _memoryAreaDataCorruptionDetermined(false)
    , _busOffCount{}
    , _failureActive{}
    , _activeSinceUs{}
{}

bool CanBusOffPolicy::onControllerBusOff(uint8_t const controllerId)
{
    if (!isValidController(controllerId))
    {
        return false;
    }

    // Any new bus-off occurrence invalidates an in-progress continuous-ACTIVE
    // recovery timer.
    _activeSinceUs[controllerId] = 0U;

    if (_busOffCount[controllerId] < _tripCount)
    {
        ++_busOffCount[controllerId];
    }

    bool const justTripped = (_busOffCount[controllerId] >= _tripCount)
        && !_failureActive[controllerId] && canDeclareOrRecover();

    if (justTripped)
    {
        _failureActive[controllerId] = true;
    }

    return justTripped;
}

bool CanBusOffPolicy::onControllerActive(uint8_t const controllerId, uint32_t const nowUs)
{
    if (!isValidController(controllerId))
    {
        return false;
    }

    if (_activeSinceUs[controllerId] == 0U)
    {
        _activeSinceUs[controllerId] = (nowUs == 0U) ? 1U : nowUs;
        return false;
    }

    if (!_failureActive[controllerId] && (_busOffCount[controllerId] == 0U))
    {
        // Nothing to recover from; keep the timer running but there is no
        // pending failure or count to clear.
        return false;
    }

    uint32_t const elapsedUs = nowUs - _activeSinceUs[controllerId];
    if (canDeclareOrRecover() && (elapsedUs >= _recoveryActiveUs))
    {
        return resetController(controllerId);
    }

    return false;
}

void CanBusOffPolicy::onControllerInactive(uint8_t const controllerId)
{
    if (!isValidController(controllerId))
    {
        return;
    }

    _activeSinceUs[controllerId] = 0U;
}

bool CanBusOffPolicy::resetController(uint8_t const controllerId)
{
    if (!isValidController(controllerId))
    {
        return false;
    }

    bool const wasActive = _failureActive[controllerId];

    _busOffCount[controllerId]   = 0U;
    _failureActive[controllerId] = false;
    _activeSinceUs[controllerId] = 0U;

    return wasActive;
}

void CanBusOffPolicy::resetAll()
{
    for (uint8_t i = 0U; i < _numControllers; ++i)
    {
        (void)resetController(i);
    }
}

uint8_t CanBusOffPolicy::getBusOffCount(uint8_t const controllerId) const
{
    return isValidController(controllerId) ? _busOffCount[controllerId] : 0U;
}

bool CanBusOffPolicy::isFailureActive(uint8_t const controllerId) const
{
    return isValidController(controllerId) ? _failureActive[controllerId] : false;
}

} // namespace canBusOff
