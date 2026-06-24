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

#include "routing/Header.h"

#include <etl/span.h>
#include <etl/unaligned_type.h>
#include <ip/IPAddress.h>

#include <platform/estdint.h>

namespace routing
{
struct PduTransportConfig
{
    enum class Mode : uint8_t
    {
        RX    = 0x01,
        TX    = 0x10,
        RX_TX = 0x11
    };

    uint8_t channelId                            = INVALID_CHANNEL_ID;
    ::ip::IPAddress ipAddress                    = {};
    ::etl::be_uint16_t vlanId                    = {};
    ::etl::be_uint16_t localPort                 = {};
    ::etl::be_uint16_t remotePort                = {};
    ::etl::be_uint16_t transmissionTimeout       = {};
    Mode mode                                    = Mode::RX_TX;
    uint8_t type                                 = 0;
    uint8_t pcp                                  = 0;
    ::etl::span<uint8_t const> remoteIpAddresses = {};
};

/**
 * Loads the PDU transport channel config from mem.
 * Returns true on success and false on error.
 * On failure, the config may be partially modified and must not be used.
 */
bool load(::etl::span<uint8_t const> mem, PduTransportConfig& config);
} // namespace routing
