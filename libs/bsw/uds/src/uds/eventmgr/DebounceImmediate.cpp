/********************************************************************************
 * Copyright (c) 2026 Contributors to the Eclipse Foundation
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#include "uds/eventmgr/DebounceImmediate.h"

namespace uds
{

DebounceImmediate::DebounceImmediate() : _currentStatus(EventStatus::PREPASSED) {}

EventStatus DebounceImmediate::update(bool isFailed, uint16_t elapsedMs)
{
    (void)elapsedMs; // Not used

    _currentStatus = isFailed ? EventStatus::FAILED : EventStatus::PASSED;
    return _currentStatus;
}

void DebounceImmediate::reset() { _currentStatus = EventStatus::PREPASSED; }

} // namespace uds
