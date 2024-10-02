// Copyright 2024 Accenture.

#ifndef GUARD_87B9D057_DD13_41D5_9BF5_02AEDAE43894
#define GUARD_87B9D057_DD13_41D5_9BF5_02AEDAE43894

#include "util/stream/IOutputStream.h"

#include <estd/algorithm.h>
#include <estd/slice.h>

#include <cstddef>
#include <cstdint>

namespace util
{
namespace stream
{
class ByteBufferOutputStream : public IOutputStream
{
public:
    /**
     * Creates a new ByteBufferOutputStream using the data contained in the provided buffer.
     *
     * \param buffer for which a associated ByteBufferOutputStream shall be created.
     */
    explicit ByteBufferOutputStream(::estd::slice<uint8_t> const& buf) : _buffer(buf) {}

    /**
     * Indicates whether or not the the underlying data buffer is full.
     *
     * \return
     * - true if the sink is full
     * - false otherwise
     */
    bool isEof() const override;
    /**
     * Writes data to the stream.
     * \note The underlying buffer will only filled up to its maximum size and thus no
     * exception will occur on writing beyond the buffer's limit. Anyway the position will
     * be adjusted as if the buffer was unlimited.
     *
     * \param data data to write to the buffer.
     */
    void write(uint8_t data) override;
    /**
     * Writes the bytes of a constant buffer to the underlying buffer.
     * \note The underlying buffer will only filled up to its maximum size and thus no
     * exception will occur on writing beyond the buffer's limit. Anyway the position will
     * be adjusted as if the buffer was unlimited.
     *
     * \param buffer data to write to the buffer
     */
    void write(::estd::slice<uint8_t const> const& buffer) override;

    /**
     * Moves the current position ahead 'blocksToSkip' if possible.
     *
     * \param bytesToSkip The number of block to move forward
     */
    void skip(size_t const bytesToSkip) { _position += bytesToSkip; }

    /**
     * Check whether an overflow has happened during writing the buffer (i.e. the position
     * in the stream is bigger than size of the underlying buffer)
     *
     * \return
     * - true if an overflow has happened
     * - false otherwise
     */
    bool isOverflow() const { return _position > _buffer.size(); }

    /**
     * Get the number of bytes that were written to the buffer.
     * \note the position may extend the size of the underlying buffer. So in case of
     * overflow this value can be used to retrieve the required number of bytes to
     * store the complete input.
     *
     * \return The number of bytes written to the buffer
     */
    size_t getPosition() const { return _position; }

    /**
     * Get the bytes that were written to the buffer.
     *
     * \return Buffer that contains the bytes written to this stream. If an overflow has
     * occurred the buffer contains the amount of bytes that fit into the underlying buffer
     */
    ::estd::slice<uint8_t> getBuffer() const
    {
        return _buffer.subslice(::estd::min(_position, _buffer.size()));
    }

private:
    ::estd::slice<uint8_t> _buffer;
    /* position of the next free byte */
    size_t _position = 0;
};

} // namespace stream
} // namespace util

#endif /* GUARD_87B9D057_DD13_41D5_9BF5_02AEDAE43894 */
