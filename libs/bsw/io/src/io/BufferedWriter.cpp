/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "io/BufferedWriter.h"

namespace io
{

::etl::span<uint8_t> BufferedWriter::allocate(size_t const size)
{
    size_t const MAXIMUM_BUFFER_SIZE = _destination.maxSize();
    if (size > MAXIMUM_BUFFER_SIZE)
    {
        _size = 0;
        return {};
    }
    bool const isEnoughSpaceAvailable = _currentBuffer.size() >= size;
    if (!isEnoughSpaceAvailable)
    {
        flush();
        _currentBuffer = _destination.allocate(MAXIMUM_BUFFER_SIZE);
        if (_currentBuffer.size() < MAXIMUM_BUFFER_SIZE)
        {
            _size = 0;
            return {};
        }
    }
    _size = size;
    return _currentBuffer.first(_size);
}

void BufferedWriter::commit()
{
    _currentBuffer.advance(_size);
    _size = 0;
}

void BufferedWriter::flush()
{
    if (_currentBuffer.size() > 0)
    {
        // Trim the buffer before committing.
        (void)_destination.allocate(_destination.maxSize() - _currentBuffer.size());
    }
    _destination.commit();
    _currentBuffer = {};
}

} // namespace io
