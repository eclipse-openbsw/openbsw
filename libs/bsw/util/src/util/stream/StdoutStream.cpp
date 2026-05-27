/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/stream/StdoutStream.h"

#include "util/stream/BspStubs.h"

namespace util
{
namespace stream
{
bool StdoutStream::isEof() const { return false; }

void StdoutStream::write(uint8_t const data) { putByteToStdout(data); }

void StdoutStream::write(::etl::span<uint8_t const> const& buffer)
{
    for (auto b : buffer)
    {
        putByteToStdout(b);
    }
}

} // namespace stream
} // namespace util
