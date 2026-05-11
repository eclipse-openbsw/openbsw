/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

// This is a generated file. Please do not edit it.

#pragma once

#include <platform/estdint.h>

namespace blob
{
enum class ConfigType : uint32_t
{
    ROUTING       = 0x00,
    RX_ADAPTER    = 0x01,
    TX_ADAPTER    = 0x02,
    CHANNEL       = 0xCC,
    CHANNEL_NAMES = 0xDD,
    META          = 0xFE,
    UNKNOWN       = 0xFFFFFFFF
};
} // namespace blob
