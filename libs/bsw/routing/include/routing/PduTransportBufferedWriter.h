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

#include "io/IWriter.h"
#include "routing/util.h"

#include <etl/limits.h>
#include <etl/span.h>

#include <platform/estdint.h>

namespace routing
{
/**
 * Class for writing smaller chunks of bytes to a destination capable of transmitting
 * larger pieces of data. The destination is meant to be the output queue of a pduTransport channel
 * with a transmission timeout. Therefore, an instance of PduTransportBufferedWriter keeps track of
 * the timestamp of the creation of the payload of a frame.
 *
 * A PduTransportBufferedWriter will always allocate destination.max_size() bytes as a buffer and
 * then use this to give away chunks of data when allocating from it. Since the size of a PDU
 * varies, it makes sense to pack multiple PDUs into one Ethernet frame to minimize the protocol
 * overhead.
 *
 * When an allocation cannot guarantee destination.max_size() bytes, it flushes the current buffer
 * and allocates a new full sized chunk from the destination. A flush writes the data to the
 * destination. Hence, the current buffer is also flushed if the transmission timeout expires.
 */
class PduTransportBufferedWriter : public ::io::IWriter
{
public:
    // [PUBLIC_API_BEGIN]
    /**
     * Constructs a PduTransportBufferedWriter from a given IWriter destination.
     */
    explicit PduTransportBufferedWriter(IWriter& destination, uint16_t transmissionTimeoutInMs);

    /**
     * \see IWriter::maxSize()
     */
    size_t maxSize() const override;

    /**
     * Allocates a span of bytes of a given size.
     *
     * If not enough memory is available in the current buffer, it will be flushed and a
     * new allocation of destination.max_size() bytes will be done.
     *
     * \param size  Number of bytes to allocate from this BufferedWriter.
     * \return  - Empty span, if requested size was greater as MAX_ELEMENT_SIZE or no memory
     *            is available
     *          - Span of bytes otherwise.
     */
    ::etl::span<uint8_t> allocate(size_t size) override;

    /**
     * Commits the previously allocated data and update the timestamp if this is first chunk
     * of the current buffer. Unless the transmission timeout is equal to zero, it is not
     * guaranteed that this data is immediately available to the reader as a call to flush()
     * might be required for that.
     */
    void commit() override;

    /**
     * Commits the data currently written to a buffer allocated from the destination making it
     * available to the reader.
     */
    void flush() override;

    /**
     * Flushes the current buffer if the transmission timeout has expired since its first chunk of
     * data was committed or if it will expire in under 1ms.
     */
    void checkTransmissionTimeout();

    /**
     * Resets the writer to its initial state.
     */
    void reset();

    // statistics
    StatCounter::Type timeoutSends() const { return _timeoutSends; }

    StatCounter::Type flushes() const { return _flushes; }

    // [PUBLIC_API_END]

private:
    IWriter& _destination;
    uint32_t const _transmissionTimeout;

    ::etl::span<uint8_t> _bufferRemainder;
    bool _isBufferEmpty;
    size_t _currentPduSize;
    uint32_t _timestamp;

    StatCounter _timeoutSends;
    StatCounter _flushes;
};

inline PduTransportBufferedWriter::PduTransportBufferedWriter(
    IWriter& destination, uint16_t const transmissionTimeoutInMs)
: _destination(destination)
, _transmissionTimeout(transmissionTimeoutInMs * 1000U)
, _bufferRemainder()
, _isBufferEmpty(true)
, _currentPduSize(0)
, _timestamp(0)
, _timeoutSends()
, _flushes()
{}

inline size_t PduTransportBufferedWriter::maxSize() const { return _destination.maxSize(); }

} // namespace routing
