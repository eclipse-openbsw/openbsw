/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "blob/Blob.h"

#include "blob/util.h"

#include <etl/limits.h>

namespace blob
{
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg): Logger API uses C-style varargs.
::etl::span<uint8_t const> load(::etl::span<uint8_t const> const mem)
{
    ::blob::Header header;
    if (!::blob::checkHeader(mem, header))
    {
        return {};
    }

    size_t const blobSize = Blob::HEADER_SIZE + header.size;

    if (mem.size() < blobSize)
    {
        logger::Logger::error(
            logger::BLOB,
            "Invalid blob size (expected=%u,actual=%u)",
            static_cast<uint32_t>(blobSize),
            static_cast<uint32_t>(mem.size()));

        return {};
    }

    auto const blob   = mem.first(blobSize);
    bool hasValidData = true;
    size_t i          = 0;
    for (auto const config : ::blob::Blob(blob))
    {
        if (!::blob::checkCrc(config.data))
        {
            hasValidData = false;

            logger::Logger::error(
                logger::BLOB, "Config %u has invalid data", static_cast<uint32_t>(i));
        }
        ++i;
    }

    if (!hasValidData)
    {
        return {};
    }

    return blob;
}

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
} // namespace blob
