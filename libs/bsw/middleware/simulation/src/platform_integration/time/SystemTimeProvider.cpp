/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#include <chrono>

#include <middleware/time/SystemTimerProvider.h>

namespace middleware::time
{

uint32_t getCurrentTimeInMs()
{
    return static_cast<uint32_t>(
        std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now())
            .time_since_epoch()
            .count());
}

uint32_t getCurrentTimeInUs()
{
    return static_cast<uint32_t>(
        std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now())
            .time_since_epoch()
            .count());
}

} // namespace middleware::time
