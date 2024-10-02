// Copyright 2024 Accenture.

#include "util/stream/StdoutStream.h"

#include "util/stream/BspStubs.h"

namespace util
{
namespace stream
{
bool StdoutStream::isEof() const { return false; }

void StdoutStream::write(uint8_t const data) { putByteToStdout(data); }

void StdoutStream::write(::estd::slice<uint8_t const> const& buffer)
{
    for (auto b : buffer)
    {
        putByteToStdout(b);
    }
}

} // namespace stream
} // namespace util
