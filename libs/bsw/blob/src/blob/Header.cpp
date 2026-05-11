/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "blob/Header.h"

#include "blob/Blob.h"
#include "blob/Logger.h"

#include <etl/unaligned_type.h>
#include <util/logger/Logger.h>

namespace blob
{
namespace logger = ::util::logger;

bool checkHeader(::etl::span<uint8_t const> const blob, Header& header)
{
    if (blob.empty())
    {
        logger::Logger::error(logger::BLOB, "Error, empty blob\n");
        return false;
    }

    if (blob.size() < Blob::HEADER_SIZE)
    {
        logger::Logger::error(
            logger::BLOB,
            "Error, blob size < %u (%u)\n",
            static_cast<uint32_t>(Blob::HEADER_SIZE),
            blob.size());
        return false;
    }

    auto blobHeader                 = blob.first(Blob::HEADER_SIZE);
    constexpr uint32_t BLOB_VERSION = 1;

    header.version = blobHeader.take<::etl::be_uint32_t const>();

    if (header.version != BLOB_VERSION)
    {
        logger::Logger::error(
            logger::BLOB,
            "Error, version != %u (%u)\n",
            BLOB_VERSION,
            static_cast<uint32_t>(header.version));
        return false;
    }

    constexpr uint32_t BLOB_MAGIC_PATTERN = 0xDEADBEEFU;

    header.magic = blobHeader.take<::etl::be_uint32_t const>();

    if (header.magic != BLOB_MAGIC_PATTERN)
    {
        logger::Logger::error(
            logger::BLOB,
            "Error, magic != %x (%x)\n",
            BLOB_MAGIC_PATTERN,
            static_cast<uint32_t>(header.magic));
        return false;
    }

    header.size = blobHeader.take<::etl::be_uint32_t const>();

    return true;
}
} // namespace blob
