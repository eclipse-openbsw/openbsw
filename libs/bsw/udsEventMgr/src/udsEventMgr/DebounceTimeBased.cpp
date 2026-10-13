/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#include "udsEventMgr/DebounceTimeBased.h"

namespace uds
{

DebounceTimeBased::DebounceTimeBased()
: _failThresholdMs(0U)
, _passThresholdMs(0U)
, _failAccumulatorMs(0U)
, _passAccumulatorMs(0U)
, _currentStatus(EventStatus::PREPASSED)
, _lastSignalWasFail(false)
{}

DebounceTimeBased::DebounceTimeBased(uint16_t failThresholdMs, uint16_t passThresholdMs)
: _failThresholdMs(failThresholdMs)
, _passThresholdMs(passThresholdMs)
, _failAccumulatorMs(0U)
, _passAccumulatorMs(0U)
, _currentStatus(EventStatus::PREPASSED)
, _lastSignalWasFail(false)
{}

EventStatus DebounceTimeBased::update(bool isFailed, uint16_t elapsedMs)
{
    if (isFailed)
    {
        if (!_lastSignalWasFail)
        {
            _lastSignalWasFail = true;
            _passAccumulatorMs = 0U; // Reset pass accumulator on transition
        }
        _failAccumulatorMs += elapsedMs;
        if (_failAccumulatorMs >= _failThresholdMs)
        {
            _currentStatus     = EventStatus::FAILED;
            _failAccumulatorMs = _failThresholdMs; // Saturate
        }
        else if (_currentStatus == EventStatus::PREPASSED)
        {
            _currentStatus = EventStatus::PREFAILED;
        }
    }
    else
    {
        if (_lastSignalWasFail)
        {
            _lastSignalWasFail = false;
            _failAccumulatorMs = 0U; // Reset fail accumulator on transition
        }
        _passAccumulatorMs += elapsedMs;
        if (_passAccumulatorMs >= _passThresholdMs)
        {
            _currentStatus     = EventStatus::PASSED;
            _passAccumulatorMs = _passThresholdMs; // Saturate
        }
        else if (_currentStatus == EventStatus::FAILED || _currentStatus == EventStatus::PREFAILED)
        {
            _currentStatus = EventStatus::PREPASSED;
        }
    }

    return _currentStatus;
}

void DebounceTimeBased::reset()
{
    _failAccumulatorMs = 0U;
    _passAccumulatorMs = 0U;
    _currentStatus     = EventStatus::PREPASSED;
    _lastSignalWasFail = false;
}

} // namespace uds
