// Copyright 2024 Accenture.

#pragma once

#include "util/stream/IOutputStream.h"
#include "util/stream/TaggedOutputHelper.h"

namespace util
{
namespace stream
{
/**
 * The TaggedOutputStream class is a tagged IOutputStream implementation.
 *
 */
class TaggedOutputStream
: public IOutputStream
, private TaggedOutputHelper
{
public:
    TaggedOutputStream(IOutputStream& strm, char const* prefix, char const* suffix);

    ~TaggedOutputStream();

    bool isEof() const override;

    void write(uint8_t data) override;
    void write(::estd::slice<uint8_t const> const& buffer) override;

private:
    IOutputStream& _stream;
};

} // namespace stream
} // namespace util

