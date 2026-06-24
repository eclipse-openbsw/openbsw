/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/PduTransportTxAdapter.h"

namespace routing
{

PduTransportTxAdapter::PduTransportTxAdapter(
    ::io::IWriter& outputWriter, ErrorHandler const errorHandler)
: _outputWriter(outputWriter)
, _errorHandler(errorHandler)
, _currentPduSize(0U)
, _pduCounter()
, _byteCounter()
, _failedMemAllocPdus()
, _droppedPduCounter()
, _droppedByteCounter()
, _lastAllocationFailed(false)
{}

size_t PduTransportTxAdapter::maxSize() const { return _outputWriter.maxSize(); }

::etl::span<uint8_t> PduTransportTxAdapter::allocate(size_t size)
{
    auto const buffer = _outputWriter.allocate(size);
    if (buffer.size() != size)
    {
        _currentPduSize = 0U;

        if (!_lastAllocationFailed)
        {
            _errorHandler(ErrorHandler::StatusCode::MEM_ALLOCATION_FAILURE);
            _lastAllocationFailed = true;
        }

        ++_failedMemAllocPdus;

        ++_droppedPduCounter;
        _droppedByteCounter += static_cast<uint32_t>(size);

        return {};
    }

    _currentPduSize       = buffer.size();
    _lastAllocationFailed = false;
    return buffer;
}

void PduTransportTxAdapter::commit()
{
    if (_currentPduSize == 0U)
    {
        return;
    }

    _outputWriter.commit();

    ++_pduCounter;
    _byteCounter += static_cast<uint32_t>(_currentPduSize);

    _currentPduSize = 0U;

    return;
}

void PduTransportTxAdapter::flush() { _outputWriter.flush(); }

} // namespace routing
