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
#include "routing/TxAdapterTable.h"
#include "routing/util.h"

#include <etl/span.h>
#include <io/IWriter.h>

#include <platform/estdint.h>

namespace routing
{
/**
 * Adapt the IWriter to create PDUs of the correct size, collect statistics and report errors
 * according to the behaviour expected for a legacy bus like CAN or FlexRay.
 *
 * The contents of an incoming PDU will be offset in a larger PDU according to the TxAdapterTable.
 * The incoming PDUs contain their ID followed by the length, both as a big endian uint32_t, and
 * then the payload.
 * The outgoing PDUs contain the same four bytes of ID, four bytes of the new length, and the
 * payload is offset inside a larger one where the extra bytes are set to 0xFF.
 */
class LegacyTxAdapter : public ::io::IWriter
{
public:
    // [PUBLIC_API_BEGIN]
    /**
     * Construct a LegacyTxAdapter from a pre-filled TxAdapterTable.
     */
    LegacyTxAdapter(
        ::io::IWriter& outputWriter,
        ::routing::TxAdapterTable const& table,
        ErrorHandler errorHandler);

    /**
     * Return the maxSize() of the underlying writer.
     */
    size_t maxSize() const override;

    /**
     * Try to allocate max_size() of the underlying writer, and return a subspan of this memory.
     * This will fail if there is not at least max_size() available in the underlying writer.
     */
    ::etl::span<uint8_t> allocate(size_t size) override;

    /**
     * Find the correct message in the table, allocate, move the buffered PDU in the
     * correct place, and fill the rest with 0xFF.
     */
    void commit() override;

    /**
     * Call flush on the underlying IWriter.
     */
    void flush() override;

    // The PDU and byte counters keep track of the total amount of data written to the output queue.

    StatCounter::Type pduCounter() const { return _pduCounter; }

    StatCounter::Type byteCounter() const { return _byteCounter; }

    // The dropped PDU and byte counters store the amount of data that was not transmitted
    // because allocation or commit failed. This occurs when the output queue is full, output
    // message ID is unknown or PDU offset is invalid.
    StatCounter::Type oversizedPdus() const { return _oversizedPdus; }

    StatCounter::Type failedMemAllocPdus() const { return _failedMemAllocPdus; }

    StatCounter::Type unknownMessagePdus() const { return _unknownMessagePdus; }

    StatCounter::Type failedMemMovePdus() const { return _failedMemMovePdus; }

    StatCounter::Type droppedPduCounter() const { return _droppedPduCounter; }

    StatCounter::Type droppedByteCounter() const { return _droppedByteCounter; }

    // [PUBLIC_API_END]
private:
    ::io::IWriter& _outputWriter;
    ::routing::TxAdapterTable _table;
    ErrorHandler const _errorHandler;
    ::etl::span<uint8_t> _buffer;
    StatCounter _pduCounter;
    StatCounter _byteCounter;

    mutable StatCounter _oversizedPdus;
    mutable StatCounter _failedMemAllocPdus;
    mutable StatCounter _unknownMessagePdus;
    mutable StatCounter _failedMemMovePdus;

    StatCounter _droppedPduCounter;
    StatCounter _droppedByteCounter;
    bool _lastAllocationFailed;
};

} // namespace routing
