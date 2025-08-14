// Copyright 2024 Accenture.

#pragma once

#include "io/IWriter.h"

#include <etl/span.h>

#include <platform/estdint.h>

namespace io
{
/**
 * Class for writing smaller chunks of bytes to a destination capable of transmitting
 * larger pieces of data.
 *
 * A BufferedWriter will always allocate destination.maxSize() bytes as a buffer and then
 * use this to give away chunks of data when allocating from it. A typical example would be
 * the destination being UDP frames which are used to transport smaller e.g. CAN frames. To
 * minimize the protocol overhead it makes sense to pack multiple CAN frames into an UDP frame.
 *
 * When an allocation cannot guarantee MAX_ELEMENT_SIZE bytes it will commit the current buffer
 * and allocate a new full sized chunk from the destination.
 */
class BufferedWriter : public IWriter
{
public:
    // [PUBLIC_API_BEGIN]
    /**
     * Constructs a BufferedWriter from a given IWriter destination.
     */
    explicit BufferedWriter(IWriter& destination);

    /**
     * \see IWriter::maxSize()
     */
    size_t maxSize() const override;

    /**
     * Allocates a slice of bytes of a given size.
     *
     * If not enough memory is available in the current buffer, it will be flushed and a
     * new allocation of destination.maxSize() bytes will be done.
     *
     * \param size  Number of bytes to allocate from this BufferedWriter.
     * \return  - Empty slice, if requested size was greater as MAX_ELEMENT_SIZE or no memory
     *            is available
     *          - Slice of bytes otherwise.
     */
    ::etl::span<uint8_t> allocate(size_t size) override;

    /**
     * Commits the previously allocated data. It is not guaranteed that this data is immediately
     * available to the reader as a call to flush() might be required for that.
     */
    void commit() override;

    /**
     * Commits the data currently written to slice allocated from destination making it available to
     * the reader.
     */
    void flush() override;
    // [PUBLIC_API_END]
private:
    IWriter& _destination;
    ::etl::span<uint8_t> _currentBuffer;
    size_t _size;
};

inline BufferedWriter::BufferedWriter(IWriter& destination)
: _destination(destination), _currentBuffer(), _size(0)
{}

inline size_t BufferedWriter::maxSize() const { return _destination.maxSize(); }

} // namespace io
