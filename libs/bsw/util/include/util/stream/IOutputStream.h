// Copyright 2024 Accenture.

#pragma once

#include <estd/slice.h>

#include <cstdint>

namespace util
{
namespace stream
{
class IOutputStream
{
public:
    IOutputStream() = default;

    IOutputStream(IOutputStream const&)            = delete;
    IOutputStream& operator=(IOutputStream const&) = delete;

    /**
     * Indicates whether or not the stream ends i.e. bytes can be written.
     *
     * \return
     * - true if the end of the stream has been reached
     * - false otherwise
     */
    virtual bool isEof() const = 0;
    /**
     * Writes data to the stream.
     *
     * \param data data to write to the buffer.
     */

    virtual void write(uint8_t data)                               = 0;
    /**
     * Writes the bytes of a buffer to the stream.
     *
     * \param buffer block of bytes to write to the buffer.
     */
    virtual void write(::estd::slice<uint8_t const> const& buffer) = 0;
};

} // namespace stream
} // namespace util
