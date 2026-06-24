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

#include <blob/Config.h>

#include <etl/span.h>
#include <etl/unaligned_type.h>

namespace blob
{
/**
 * load a column of a table (routing, adapter...) from a blob config
 **/
template<class T>
::etl::span<uint8_t const>
loadColumn(::etl::span<T const>& column, ::etl::span<uint8_t const> config)
{
    if (config.size() < sizeof(::etl::be_uint32_t))
    {
        column = {};
        return {};
    }

    auto const size = config.take<::etl::be_uint32_t const>();
    if ((config.size() < size) || (size % sizeof(T) != 0))
    {
        column = {};
        return {};
    }

    column = config.take<uint8_t const>(size).reinterpret_as<T const>();
    return config;
}

::blob::Config config(::etl::span<uint8_t const> blob, ::blob::Config::Type type);

// [CrcCheck]
bool checkCrc(::etl::span<uint8_t const> config);
// [CrcCheck]
} // namespace blob
