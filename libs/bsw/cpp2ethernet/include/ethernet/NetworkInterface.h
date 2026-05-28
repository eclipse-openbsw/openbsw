/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <platform/estdint.h>

namespace ethernet
{
struct NETIF_CONFIGURED
{};

struct NETIF_INITIALISED
{};

struct NETIF_STARTED
{};

struct NETIF_UP
{};

struct NetifBusId
{
    uint8_t value = 0;

    operator uint8_t() const { return value; }
};

struct NetifPort
{
    uint8_t value = 0;
};

struct NetifVlanId
{
    uint16_t value = 0;
};

enum class NetifState
{
    Uninitialised,
    Initialised,
    Started
};

enum class LinkStatus
{
    Down,
    Up
};

} // namespace ethernet
