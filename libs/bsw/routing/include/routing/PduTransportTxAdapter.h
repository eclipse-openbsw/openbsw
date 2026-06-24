/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "routing/ErrorHandler.h"
#include "routing/util.h"

#include <etl/span.h>
#include <io/IWriter.h>

#include <platform/estdint.h>

namespace routing
{
/**
 * Adapts an IWriter to count sent pdus and bytes.
 */
class PduTransportTxAdapter : public ::io::IWriter
{
public:
    // [PUBLIC_API_BEGIN]
    /**
     * All the methds delegate to the underlying IWriter, while counting the bytes.
     */
    PduTransportTxAdapter(::io::IWriter& outputWriter, ErrorHandler errorHandler);

    size_t maxSize() const override;

    ::etl::span<uint8_t> allocate(size_t size) override;

    void commit() override;

    void flush() override;

    // The PDU and byte counters keep track of the total amount of data written to the output queue.
    StatCounter::Type pduCounter() const { return _pduCounter; }

    StatCounter::Type byteCounter() const { return _byteCounter; }

    // The dropped PDU and byte counters store the amount of data that was not transmitted
    // because the output queue was full.
    StatCounter::Type failedMemAllocPdus() const { return _failedMemAllocPdus; }

    StatCounter::Type droppedPduCounter() const { return _droppedPduCounter; }

    StatCounter::Type droppedByteCounter() const { return _droppedByteCounter; }

    // [PUBLIC_API_END]
private:
    ::io::IWriter& _outputWriter;
    ErrorHandler const _errorHandler;
    size_t _currentPduSize;
    StatCounter _pduCounter;
    StatCounter _byteCounter;

    mutable StatCounter _failedMemAllocPdus;

    StatCounter _droppedPduCounter;
    StatCounter _droppedByteCounter;
    bool _lastAllocationFailed;
};

} // namespace routing
