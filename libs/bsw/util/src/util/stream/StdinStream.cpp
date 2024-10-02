// Copyright 2024 Accenture.

#include "util/stream/StdinStream.h"

#include "util/stream/BspStubs.h"

namespace util
{
namespace stream
{
bool StdinStream::isEof() const { return false; }

uint32_t StdinStream::readBuffer(::estd::slice<uint8_t> const& buffer)
{
    uint32_t count = 0U;
    while (count < buffer.size())
    {
        int32_t const c = getByteFromStdin();
        if (c >= 0)
        {
            buffer[count] = static_cast<uint8_t>(c);
            ++count;
        }
        else
        {
            break;
        }
    }
    return count;
}

} // namespace stream
} // namespace util
