// Copyright 2024 Accenture.

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
