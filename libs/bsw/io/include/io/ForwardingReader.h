// Copyright 2024 Accenture.

#pragma once

#include "io/IReader.h"
#include "io/IWriter.h"

#include <estd/memory.h>

namespace io
{

/**
 * Adapter class for an IReader, that forwards all read items to an IWriter.
 *
 * \warning
 * This class will only work for single producer, single consumer use cases because the API
 * is a two step API. Calling peek() on the source will call allocate() on the destination
 * and calling release() will call commit(). If you have multiple producers, the integrator
 * needs to take care that all calls are protected against each other correctly.
 */
class ForwardingReader : public IReader
{
public:
    // [PUBLIC_API_BEGIN]
    /**
     * Constructs a ForwardingReader connecting a given source and destination.
     */
    ForwardingReader(IReader& source, IWriter& destination);

    /**
     * Returns source's maxSize().
     */
    size_t maxSize() const override;

    /**
     * Calling peek() will call peek() on the source reader and if data is available allocate
     * the data on the destination writer.
     */
    ::estd::slice<uint8_t> peek() const override;

    /**
     * Calling release() will copy the data to the destination and commit it. After that,
     * release() is called on the source to free the input data.
     */
    void release() override;
    // [PUBLIC_API_END]
    /**
     * Counter for failed allocations, when calling peek() did return data from the source
     * but allocate() on the destination failed.
     */
    mutable size_t failedAllocations{0U};

private:
    IReader& _source;
    IWriter& _destination;
    ::estd::slice<uint8_t> mutable _sourceData{};
    ::estd::slice<uint8_t> mutable _destinationData{};
};

inline ForwardingReader::ForwardingReader(IReader& source, IWriter& destination)
: _source(source), _destination(destination)
{}

inline size_t ForwardingReader::maxSize() const { return _source.maxSize(); }

inline ::estd::slice<uint8_t> ForwardingReader::peek() const
{
    _sourceData = _source.peek();

    // Only forward data, if data is available
    if (_sourceData.size() > 0)
    {
        _destinationData = _destination.allocate(_sourceData.size());
        if (_destinationData.size() == 0U)
        {
            ++failedAllocations;
        }
    }
    return _sourceData;
}

inline void ForwardingReader::release()
{
    // If _destinationData is an empty slice, copy will not copy any data.
    (void)::estd::memory::copy(_destinationData, _sourceData);
    // Calling commit() will only have an effect if allocate was called before, this is why
    // we don't need more checks here and can just call commit().
    _destination.commit();
    // Invalidate the slices so that calling release() multiple times doesn't copy data by
    // accident.
    _destinationData = {};
    _sourceData      = {};
    _source.release();
}

} // namespace io
