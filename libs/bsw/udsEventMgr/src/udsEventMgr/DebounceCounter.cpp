/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#include "udsEventMgr/DebounceCounter.h"
#include "udsEventMgr/IDebounceAlgorithm.h"
#include <cstdint>

namespace uds
{

DebounceCounter::DebounceCounter()
: _failThreshold(1U), _passThreshold(1U), _counter(0U), _currentStatus(EventStatus::PREPASSED)
{}

DebounceCounter::DebounceCounter(uint16_t failThreshold, uint16_t passThreshold)
: _failThreshold(failThreshold)
, _passThreshold(passThreshold)
, _counter(0U)
, _currentStatus(EventStatus::PREPASSED)
{}

EventStatus DebounceCounter::update(bool isFailed, [[maybe_unused]] uint16_t elapsedMs)
{
    if (isFailed)
    {
        if (_currentStatus == EventStatus::PREPASSED || _currentStatus == EventStatus::PASSED)
        {
            _counter       = 0U;
            _currentStatus = EventStatus::PREFAILED;
        }
        if (_currentStatus == EventStatus::PREFAILED)
        {
            ++_counter;
            if (_counter >= _failThreshold)
            {
                _currentStatus = EventStatus::FAILED;
            }
        }
    }
    else
    {
        if (_currentStatus == EventStatus::PREFAILED || _currentStatus == EventStatus::FAILED)
        {
            _counter       = 0U;
            _currentStatus = EventStatus::PREPASSED;
        }
        if (_currentStatus == EventStatus::PREPASSED)
        {
            ++_counter;
            if (_counter >= _passThreshold)
            {
                _currentStatus = EventStatus::PASSED;
            }
        }
    }

    return _currentStatus;
}

void DebounceCounter::reset()
{
    _counter       = 0U;
    _currentStatus = EventStatus::PREPASSED;
}

} // namespace uds
