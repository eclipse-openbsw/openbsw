// Copyright 2024 Accenture.

#include "util/stream/StringBufferOutputStream.h"

#include <estd/limits.h>
#include <estd/memory.h>
#include <estd/string.h>

#include <cstring>

namespace util
{
namespace stream
{
StringBufferOutputStream::StringBufferOutputStream(
    ::estd::slice<char> const buf, char const* const endOfString, char const* const ellipsis)
: IOutputStream()
, _buffer(buf)
, _endOfString((endOfString != nullptr) ? endOfString : "")
, _ellipsis((ellipsis != nullptr) ? ellipsis : "")
, _currentIndex(0U)
, _overflow(false)
{}

StringBufferOutputStream::~StringBufferOutputStream() { (void)getString(); }

bool StringBufferOutputStream::isEof() const { return (_currentIndex + 1U) >= _buffer.size(); }

void StringBufferOutputStream::write(uint8_t const data)
{
    if (_currentIndex < _buffer.size())
    {
        _buffer[_currentIndex] = static_cast<char>(data);
        ++_currentIndex;
    }
}

void StringBufferOutputStream::write(::estd::slice<uint8_t const> const& buffer)
{
    auto trimmedBuffer = buffer;
    size_t size        = buffer.size();
    if ((_currentIndex + size) > _buffer.size())
    {
        size      = _buffer.size() - _currentIndex;
        _overflow = true;
        trimmedBuffer.trim(size);
    }
    (void)::estd::memory::copy(
        _buffer.offset(_currentIndex).subslice(size * sizeof(char)).reinterpret_as<uint8_t>(),
        trimmedBuffer);
    _currentIndex += size;
}

::estd::slice<char> StringBufferOutputStream::getBuffer()
{
    char const* const tempString = getString();
    size_t const tempSize        = _buffer.size();
    return _buffer.subslice(::estd::strnlen(tempString, tempSize) + 1U);
}

void StringBufferOutputStream::reset()
{
    _currentIndex = 0U;
    _overflow     = false;
}

char const* StringBufferOutputStream::getString()
{
    auto const dataBuffer = _buffer.reinterpret_as<uint8_t>();
    size_t const eolLen   = ::estd::strnlen(_endOfString, _buffer.size()) + 1U;
    if (_overflow || ((eolLen + _currentIndex) > _buffer.size()))
    {
        size_t const ellipsisLen = ::estd::strnlen(_ellipsis, _buffer.size());
        _currentIndex            = _buffer.size() - (eolLen + ellipsisLen);
        (void)::estd::memory::copy(
            dataBuffer.offset(_currentIndex).subslice(ellipsisLen),
            ::estd::slice<char const>::from_pointer(_ellipsis, ellipsisLen)
                .reinterpret_as<uint8_t const>());
        _currentIndex += ellipsisLen;
    }
    (void)::estd::memory::copy(
        dataBuffer.offset(_currentIndex).subslice(eolLen),
        ::estd::slice<char const>::from_pointer(_endOfString, eolLen)
            .reinterpret_as<uint8_t const>());
    return _buffer.data();
}

} // namespace stream
} // namespace util
