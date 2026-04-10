// Copyright 2024 Accenture.

#include "util/stream/StringBufferOutputStream.h"

#include <etl/char_traits.h>
#include <etl/limits.h>
#include <etl/memory.h>
#include <etl/span.h>
#include <etl/string.h>

#include <cstring>

namespace util
{
namespace stream
{
namespace
{
size_t boundedStringLength(char const* const value, size_t const maxLength) noexcept
{
    size_t length = 0U;
    while ((length < maxLength) && (value[length] != '\0'))
    {
        ++length;
    }
    return length;
}
} // namespace

StringBufferOutputStream::StringBufferOutputStream(
    ::etl::span<char> const buf, char const* const endOfString, char const* const ellipsis)
: IOutputStream()
, _buffer(buf)
, _endOfString((endOfString != nullptr) ? endOfString : "")
, _ellipsis((ellipsis != nullptr) ? ellipsis : "")
, _currentIndex(0U)
, _overflow(false)
{}

StringBufferOutputStream::~StringBufferOutputStream() noexcept { finalizeBuffer(); }

bool StringBufferOutputStream::isEof() const { return (_currentIndex + 1U) >= _buffer.size(); }

void StringBufferOutputStream::write(uint8_t const data)
{
    if (_currentIndex < _buffer.size())
    {
        _buffer[_currentIndex] = static_cast<char>(data);
        ++_currentIndex;
    }
}

void StringBufferOutputStream::write(::etl::span<uint8_t const> const& buffer)
{
    auto trimmedBuffer = buffer;
    size_t size        = buffer.size();
    if ((_currentIndex + size) > _buffer.size())
    {
        size          = _buffer.size() - _currentIndex;
        _overflow     = true;
        trimmedBuffer = trimmedBuffer.first(size);
    }
    (void)::etl::copy(
        trimmedBuffer, _buffer.subspan(_currentIndex, size).reinterpret_as<uint8_t>());
    _currentIndex += size;
}

::etl::span<char> StringBufferOutputStream::getBuffer()
{
    char const* const tempString = getString();
    size_t const tempSize        = _buffer.size();
    return _buffer.first(::etl::strlen(tempString, tempSize) + 1U);
}

void StringBufferOutputStream::reset()
{
    _currentIndex = 0U;
    _overflow     = false;
}

void StringBufferOutputStream::finalizeBuffer() noexcept
{
    size_t const bufferSize = _buffer.size();
    if (bufferSize == 0U)
    {
        return;
    }

    size_t const endOfStringLen = boundedStringLength(_endOfString, bufferSize - 1U);
    size_t const requiredSuffix = endOfStringLen + 1U;
    size_t writeIndex           = _currentIndex;

    if (_overflow || ((writeIndex + requiredSuffix) > bufferSize))
    {
        size_t const availableForEllipsis = bufferSize - requiredSuffix;
        size_t const ellipsisLen          = boundedStringLength(_ellipsis, availableForEllipsis);
        writeIndex                        = availableForEllipsis - ellipsisLen;

        if (ellipsisLen > 0U)
        {
            (void)::etl::mem_copy(_ellipsis, ellipsisLen, _buffer.data() + writeIndex);
            writeIndex += ellipsisLen;
        }

        _currentIndex = writeIndex;
    }

    if (endOfStringLen > 0U)
    {
        (void)::etl::mem_copy(_endOfString, endOfStringLen, _buffer.data() + writeIndex);
    }

    _buffer[writeIndex + endOfStringLen] = '\0';
}

char const* StringBufferOutputStream::getString()
{
    finalizeBuffer();
    return _buffer.data();
}

} // namespace stream
} // namespace util
