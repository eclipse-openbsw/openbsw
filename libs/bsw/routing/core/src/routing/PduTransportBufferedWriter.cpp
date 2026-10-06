/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/PduTransportBufferedWriter.h"

#include <bsp/timer/SystemTimer.h>
#include <io/IWriter.h>

#include <limits>

namespace routing
{

::etl::span<uint8_t> PduTransportBufferedWriter::allocate(size_t const size)
{
    size_t const MAXIMUM_BUFFER_SIZE = _destination.maxSize();
    if (size > MAXIMUM_BUFFER_SIZE)
    {
        _currentPduSize = 0;
        return {};
    }

    bool const isEnoughSpaceAvailable = _bufferRemainder.size() >= size;
    if (isEnoughSpaceAvailable)
    {
        _currentPduSize = size;
        return _bufferRemainder.first(_currentPduSize);
    }

    // If some data has already been committed, the buffer must be flushed prior to allocation.
    if (!_isBufferEmpty)
    {
        flush();
    }

    _bufferRemainder = _destination.allocate(MAXIMUM_BUFFER_SIZE);
    if (_bufferRemainder.size() != MAXIMUM_BUFFER_SIZE)
    {
        _currentPduSize = 0;
        return {};
    }

    _currentPduSize = size;
    return _bufferRemainder.first(_currentPduSize);
}

void PduTransportBufferedWriter::commit()
{
    if (_currentPduSize == 0)
    {
        return;
    }

    if (_bufferRemainder.size() == _destination.maxSize())
    {
        // First chunk
        _timestamp     = getSystemTimeUs32Bit();
        _isBufferEmpty = false;
    }

    _bufferRemainder.advance(_currentPduSize);
    _currentPduSize = 0;

    if (_transmissionTimeout == 0)
    {
        ++_timeoutSends;
        flush();
    }
}

void PduTransportBufferedWriter::flush()
{
    if (_isBufferEmpty)
    {
        return;
    }

    if (!_bufferRemainder.empty())
    {
        // Trim the buffer before committing.
        (void)_destination.allocate(_destination.maxSize() - _bufferRemainder.size());
    }
    ++_flushes;
    _destination.commit();

    reset();
}

void PduTransportBufferedWriter::checkTransmissionTimeout()
{
    // If transmission timeout is equal to zero, the buffer is always empty after commit.
    if (_isBufferEmpty)
    {
        return;
    }

    // Transmission timeout is greater than zero and a multiple of 1000.
    if ((getSystemTimeUs32Bit() - _timestamp) > (_transmissionTimeout - 1000U))
    {
        ++_timeoutSends;
        flush();
    }
}

void PduTransportBufferedWriter::reset()
{
    _bufferRemainder = {};
    _isBufferEmpty   = true;
    _currentPduSize  = 0;
}

} // namespace routing
