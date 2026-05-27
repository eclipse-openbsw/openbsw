/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/stream/TaggedOutputStream.h"

#include <etl/span.h>

namespace util
{
namespace stream
{
TaggedOutputStream::TaggedOutputStream(
    IOutputStream& strm, char const* const prefix, char const* const suffix)
: IOutputStream(), TaggedOutputHelper(prefix, suffix), _stream(strm)
{}

TaggedOutputStream::~TaggedOutputStream() { endLine(_stream); }

bool TaggedOutputStream::isEof() const { return _stream.isEof(); }

void TaggedOutputStream::write(uint8_t const data)
{
    writeBytes(_stream, ::etl::span<uint8_t const>(&data, 1));
}

void TaggedOutputStream::write(::etl::span<uint8_t const> const& buffer)
{
    writeBytes(_stream, buffer);
}

} // namespace stream
} // namespace util
