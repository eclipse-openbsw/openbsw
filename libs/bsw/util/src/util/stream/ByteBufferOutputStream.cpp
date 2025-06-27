// Copyright 2024 Accenture.

#include "util/stream/ByteBufferOutputStream.h"

#include <etl/memory.h>
#include <etl/span.h>

namespace util
{
namespace stream
{
bool ByteBufferOutputStream::isEof() const { return _position >= _buffer.size(); }

void ByteBufferOutputStream::write(uint8_t const data)
{
    if (_position < _buffer.size())
    {
        _buffer[_position] = data;
    }
    ++_position;
}

void ByteBufferOutputStream::write(::etl::span<uint8_t const> const& buffer)
{
    if (_position < _buffer.size())
    {
        size_t const bytesToCopy = ((_position + buffer.size()) <= _buffer.size())
                                       ? buffer.size()
                                       : (_buffer.size() - _position);

        (void)::etl::mem_copy(buffer.data(), bytesToCopy, _buffer.data() + _position);
    }
    _position += buffer.size();
}

} // namespace stream
} // namespace util
