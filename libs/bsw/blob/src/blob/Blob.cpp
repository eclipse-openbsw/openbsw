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
::etl::span<uint8_t const> load(::etl::span<uint8_t const> const mem)
{
    if (mem.size() < Blob::HEADER_SIZE)
    {
        logger::Logger::error(logger::BLOB, "Invalid memory region\n");

        return {};
    }

    auto const blobHeader = mem.first(Blob::HEADER_SIZE);

    ::blob::Header header;

    if (!::blob::checkHeader(blobHeader, header))
    {
        return {};
    }

    size_t const blobSize = Blob::HEADER_SIZE + header.size;

    if (mem.size() < blobSize)
    {
        logger::Logger::error(logger::BLOB, "Invalid blob size\n");

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

            logger::Logger::error(logger::BLOB, "Error, config no. %u has invalid data\n", i);
        }
        ++i;
    }

    if (!hasValidData)
    {
        return {};
    }

    return blob;
}
} // namespace blob
