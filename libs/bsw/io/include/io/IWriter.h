// Copyright 2024 Accenture.

#pragma once

#include <etl/span.h>

namespace io
{
/**
 * Interface for writing slices of bytes with variable size to a data channel.
 */
class IWriter
{
public:
    virtual ~IWriter() = default;

    IWriter& operator=(IWriter const&) = delete;

    // [PUBLICAPI_START]
    /**
     * Returns the maximum number of bytes that can be allocated in one allocation
     */
    virtual size_t maxSize() const = 0;

    /**
     * Allocates a slice of a given number of bytes.
     * \return - Slice of size bytes if underlying queue was not full and size <= maxSize()
     *         - Empty slice otherwise
     *
     * Calling allocate multiple times before calling commit will re-allocate, i.e. a new slice of
     * bytes will be returned and the previous allocation is lost and will not be automatically
     * committed. This can also be used to trim the allocated slice to the right size before
     * committing, thus calling allocate will NOT modify the memory (e.g. zero it).
     */
    virtual ::etl::span<uint8_t> allocate(size_t size) = 0;

    /**
     * Makes the previously allocated slice available for the reader.
     *
     * Calling commit() makes a new allocation mandatory! The previously allocated slice becomes
     * invalid and must not be used anymore. Calling commit() without a previous successful
     * allocation has no effect.
     */
    virtual void commit() = 0;

    /**
     * Flushes any buffered data which has not yet been sent.
     *
     * flush() is part of the interface to make subclassing, that implements buffering
     * to e.g. improve bandwidth usage possible. It might have an empty implementation.
     */
    virtual void flush() = 0;
    // [PUBLICAPI_END]
};

} // namespace io
