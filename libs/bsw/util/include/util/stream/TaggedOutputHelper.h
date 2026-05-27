/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "util/stream/IOutputStream.h"

#include <etl/span.h>
#include <etl/string_view.h>

namespace util
{
namespace stream
{
/**
 * The TaggedOutputHelper class provides a set of functions
 * needed for tagged output, namely for proper outputting
 * desired prefix and suffix information.
 *
 */
class TaggedOutputHelper
{
public:
    TaggedOutputHelper(char const* prefix, char const* suffix);

    void reset();
    bool isLineStart() const;
    void writeBytes(IOutputStream& strm, ::etl::span<uint8_t const> const& buffer);
    void writeBytes(IOutputStream& strm, ::etl::string_view const& view);
    void endLine(IOutputStream& strm);

private:
    void writeByte(IOutputStream& strm, uint8_t data);

    char const* _prefix;
    char const* _suffix;
    bool _lineStart;
};

} // namespace stream
} // namespace util
