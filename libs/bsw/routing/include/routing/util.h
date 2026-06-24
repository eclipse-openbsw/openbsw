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

#include "routing/ClampedCounter.h"
#include "routing/Header.h"

#include <blob/Config.h>
#include <etl/span.h>

#include <platform/estdint.h>

namespace routing
{
using StatCounter = ClampedCounter<uint32_t>;

::blob::Config config(
    ::etl::span<uint8_t const> blob,
    ::blob::Config::Type type,
    uint8_t channelId = ::routing::INVALID_CHANNEL_ID);

} // namespace routing
