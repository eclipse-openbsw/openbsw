// Copyright 2024 Accenture.

#pragma once

#include "util/stream/IOutputStream.h"

#include <estd/slice.h>

namespace util
{
namespace stream
{
/**
 * Specific IOutputStream class implementation.
 * A lightweight output stream similar to std::stringstream.
 * This class provides an interface to write formatted text into a buffer.
 */
class StringBufferOutputStream : public IOutputStream
{
public:
    explicit StringBufferOutputStream(
        ::estd::slice<char> buf, char const* endOfString = nullptr, char const* ellipsis = nullptr);
    ~StringBufferOutputStream();

    bool isEof() const override;
    void write(uint8_t data) override;
    void write(::estd::slice<uint8_t const> const& buffer) override;

    void reset();

    ::estd::slice<char> getBuffer();
    char const* getString();

private:
    ::estd::slice<char> _buffer;
    char const* _endOfString;
    char const* _ellipsis;
    size_t _currentIndex;
    bool _overflow;
};

namespace declare
{
template<size_t N>
class StringBufferOutputStream : public ::util::stream::StringBufferOutputStream
{
public:
    explicit StringBufferOutputStream(
        char const* const endOfString = nullptr, char const* const ellipsis = nullptr)
    : ::util::stream::StringBufferOutputStream(_buffer, endOfString, ellipsis)
    {}

private:
    char _buffer[N];
};

} // namespace declare
} // namespace stream
} // namespace util
