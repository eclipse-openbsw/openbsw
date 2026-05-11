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
#include "routing/RxAdapterTable.h"
#include "routing/util.h"

#include <etl/span.h>
#include <io/IReader.h>

#include <platform/estdint.h>

namespace routing
{
/**
 * Extracts the PDUs contained in the messages read from inputReader, adapting the IReader
 * interface, according to the provided RxAdapterTable.
 */
class RxAdapter : public ::io::IReader
{
public:
    static constexpr size_t MESSAGE_HEADER_SIZE = 8U;

    // [PUBLIC_API_BEGIN]
    /**
     * Construct an RxAdapter with a pre-filled RxAdapterTable.
     */
    RxAdapter(
        ::io::IReader& inputReader,
        ::etl::span<uint8_t> buffer,
        RxAdapterTable const& table,
        ErrorHandler errorHandler);

    /**
     * Construct an RxAdapter from the provided config byte array.
     */
    RxAdapter(
        ::io::IReader& inputReader,
        ::etl::span<uint8_t> buffer,
        ::etl::span<uint8_t const> config,
        ErrorHandler errorHandler);

    /**
     * Return the size of the buffer.
     */
    size_t maxSize() const override;

    /**
     * Each peek() call following a release() call will return the next PDU extracted from the
     * current message of the IReader, according to the list defined in the RxAdapterTable.
     * Once all the PDUs have been read, or if the current message is not known in the table, a new
     * peek() call will be performed on the IReader, releasing the previous one.
     *
     * An empty span will be returned if nothing is available.
     */
    ::etl::span<uint8_t> peek() const override;

    /**
     * Reset the internal PDU buffer, allowing the next peek() call to extract a new PDU from a
     * message.
     */
    void release() override;

    // The PDU and byte counters keep track of the total amount of data read from the input queue,
    // regardless of the number of PDUs extracted.
    StatCounter::Type pduCounter() const { return _pduCounter; }

    StatCounter::Type byteCounter() const { return _byteCounter; }

    // The discarded PDU and byte counters store the amount of data in incoming messages out of
    // which not a single PDU was extracted.
    StatCounter::Type invalidHeaderSizePdus() const { return _invalidHeaderSizePdus; }

    StatCounter::Type unknownMessagePdus() const { return _unknownMessagePdus; }

    StatCounter::Type invalidParsedPayloadLengthPdus() const
    {
        return _invalidParsedPayloadLengthPdus;
    }

    StatCounter::Type invalidPayloadLengthPdus() const { return _invalidPayloadLengthPdus; }

    StatCounter::Type discardedPduCounter() const { return _discardedPduCounter; }

    StatCounter::Type discardedByteCounter() const { return _discardedByteCounter; }

    // [PUBLIC_API_END]

protected:
    virtual ErrorHandler::StatusCode lookup(::etl::span<uint8_t const> message) const;

    bool extract(
        ::etl::span<uint8_t const> message,
        size_t channelSpecificPduId,
        ::etl::span<uint8_t>& pdu) const;

    ::io::IReader& _inputReader;
    ::etl::span<uint8_t> const _buffer;
    RxAdapterTable _table;

    ErrorHandler const _errorHandler;

    mutable size_t _currentMessageIndex;
    mutable size_t _currentPduIndex;
    mutable ::etl::span<uint8_t> _currentPdu;

    mutable StatCounter _pduCounter;
    mutable StatCounter _byteCounter;

    mutable StatCounter _invalidHeaderSizePdus;
    mutable StatCounter _unknownMessagePdus;
    mutable StatCounter _invalidParsedPayloadLengthPdus;
    mutable StatCounter _invalidPayloadLengthPdus;

    mutable StatCounter _discardedPduCounter;
    mutable StatCounter _discardedByteCounter;
};

} // namespace routing
