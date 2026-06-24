/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "blob/Config.h"

#include "blob/Logger.h"

#include <etl/limits.h>
#include <etl/unaligned_type.h>

namespace blob
{
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg): Logger API uses C-style varargs.
Config::Header loadConfigHeader(::etl::span<uint8_t const>& mem)
{
    if (mem.size() < sizeof(Config::Header))
    {
        return {};
    }

    Config::Header header;
    header.configType
        = static_cast<Config::Type>(static_cast<uint32_t>(mem.take<::etl::be_uint32_t const>()));
    header.reserved = mem.take<::etl::be_uint64_t const>();
    header.size     = mem.take<::etl::be_uint32_t const>();

    return header;
}

Config Config::from_bytes(::etl::span<uint8_t const> const mem)
{
    auto data = mem;

    auto const header = loadConfigHeader(data);

    if (header.configType == Type::UNKNOWN)
    {
        ::util::logger::Logger::error(::util::logger::BLOB, "Unknown config type");
        return {};
    }

    if (header.size > ::etl::numeric_limits<uint32_t>::max() - sizeof(header))
    {
        ::util::logger::Logger::error(
            ::util::logger::BLOB,
            "Config header too big: size=%u exceeds maximum",
            static_cast<uint32_t>(header.size));
        return {};
    }

    auto const size = sizeof(header) + header.size;
    if (mem.size() < size)
    {
        ::util::logger::Logger::error(
            ::util::logger::BLOB,
            "Invalid config size (expected=%u,actual=%u)",
            static_cast<uint32_t>(size),
            static_cast<uint32_t>(mem.size()));
        return {};
    }
    return {header.configType, mem.first(size)};
}

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
} // namespace blob
