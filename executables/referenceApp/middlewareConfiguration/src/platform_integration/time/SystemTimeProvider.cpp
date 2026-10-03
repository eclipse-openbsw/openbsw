/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#include <bsp/timer/SystemTimer.h>

#include <middleware/time/SystemTimerProvider.h>

namespace middleware::time
{

uint32_t getCurrentTimeInMs() { return getSystemTimeMs32Bit(); }

uint32_t getCurrentTimeInUs() { return getSystemTimeUs32Bit(); }

} // namespace middleware::time
