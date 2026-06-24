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

#include "blob/ConfigType.h"

#include <etl/span.h>

namespace blob
{
struct Config
{
    using Type = ::blob::ConfigType;

    struct __attribute__((packed)) Header
    {
        Type configType   = Type::UNKNOWN;
        uint64_t reserved = 0U;
        uint32_t size     = 0U;
    };

    bool operator==(Config const& other) const
    {
        return (type == other.type) && (data.size() == other.data.size())
               && (data.data() == other.data.data());
    }

    Config* operator->() { return this; }

    static Config from_bytes(::etl::span<uint8_t const> const mem);

    Type type = Type::UNKNOWN;
    ::etl::span<uint8_t const> data;
};

Config::Header loadConfigHeader(::etl::span<uint8_t const>& mem);

} // namespace blob
