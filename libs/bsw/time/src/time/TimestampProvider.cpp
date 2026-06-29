/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "time/TimestampProvider.h"

#include <bsp/SystemTime.h>

namespace bsw
{
namespace time
{

uint32_t TimestampProvider::getTimestampUs32Bit() { return getSystemTimeUs32Bit(); }

uint64_t TimestampProvider::getTimestampUs64Bit() { return getSystemTimeUs64Bit(); }

uint32_t TimestampProvider::getTimestampMs32Bit() { return getSystemTimeMs32Bit(); }

uint64_t TimestampProvider::getTimestampMs64Bit() { return getSystemTimeMs64Bit(); }

} // namespace time
} // namespace bsw
