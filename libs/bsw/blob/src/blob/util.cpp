/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "blob/util.h"

#include "blob/Blob.h"
#include "blob/Config.h"
#include "util/crc/Crc32.h"

#include <etl/unaligned_type.h>

namespace blob
{
::blob::Config config(::etl::span<uint8_t const> const blob, ::blob::Config::Type const type)
{
    for (auto const config : ::blob::Blob(blob))
    {
        if (config.type != type)
        {
            continue;
        }
        return config;
    }
    return {};
}

bool checkCrc(::etl::span<uint8_t const> config)
{
    constexpr size_t CRC_BYTES_SIZE = 4;

    using CrcRegister = ::util::crc::Crc32::ARE2EP4;

    if (config.size() < CRC_BYTES_SIZE)
    {
        return false;
    }

    auto const configData   = config.take<uint8_t const>(config.size() - CRC_BYTES_SIZE);
    uint32_t const crcBytes = config.take<::etl::be_uint32_t const>();

    CrcRegister crcRegister;
    crcRegister.init();
    (void)crcRegister.update(configData);

    return (crcRegister.digest() == crcBytes);
}
} // namespace blob
