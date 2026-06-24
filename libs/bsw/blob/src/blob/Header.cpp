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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg): Logger API uses C-style varargs.

bool checkHeader(::etl::span<uint8_t const> const blob, Header& header)
{
    if (blob.size() < Blob::HEADER_SIZE)
    {
        logger::Logger::error(
            logger::BLOB,
            "Invalid blob header size (expected=%u,actual=%u)",
            static_cast<uint32_t>(Blob::HEADER_SIZE),
            static_cast<uint32_t>(blob.size()));
        return false;
    }

    auto blobHeader = blob.first(Blob::HEADER_SIZE);

    header.version = blobHeader.take<::etl::be_uint32_t const>();
    if (header.version != Blob::VERSION)
    {
        logger::Logger::error(
            logger::BLOB,
            "Invalid blob version (expected=%u,actual=%u)",
            Blob::VERSION,
            static_cast<uint32_t>(header.version));
        return false;
    }

    header.magic = blobHeader.take<::etl::be_uint32_t const>();

    if (header.magic != Blob::MAGIC)
    {
        logger::Logger::error(
            logger::BLOB,
            "Invalid blob magic number (expected=0x%x,actual=0x%x)",
            Blob::MAGIC,
            static_cast<uint32_t>(header.magic));
        return false;
    }

    header.size = blobHeader.take<::etl::be_uint32_t const>();

    return true;
}

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
} // namespace blob
