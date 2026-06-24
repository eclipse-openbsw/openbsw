/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/LegacyTxAdapter.h"

#include "routing/Logger.h"

#include <etl/algorithm.h>
#include <etl/iterator.h>
#include <etl/memory.h>
#include <etl/unaligned_type.h>
#include <util/logger/Logger.h>

namespace routing
{

namespace logger = ::util::logger;

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg): Logger API uses C-style varargs.

LegacyTxAdapter::LegacyTxAdapter(
    ::io::IWriter& outputWriter, TxAdapterTable const& table, ErrorHandler const errorHandler)
: _outputWriter(outputWriter)
, _table(table)
, _errorHandler(errorHandler)
, _buffer()
, _pduCounter()
, _byteCounter()
, _oversizedPdus()
, _failedMemAllocPdus()
, _unknownMessagePdus()
, _failedMemMovePdus()
, _droppedPduCounter()
, _droppedByteCounter()
, _lastAllocationFailed(false)
{}

size_t LegacyTxAdapter::maxSize() const { return _outputWriter.maxSize(); }

::etl::span<uint8_t> LegacyTxAdapter::allocate(size_t size)
{
    _buffer = {};

    if (size > _outputWriter.maxSize())
    {
        ++_oversizedPdus;

        ++_droppedPduCounter;
        _droppedByteCounter += static_cast<uint32_t>(size);

        return {};
    }

    // Check if we can allocate the max underlying size.
    // This is because otherwise we might not have enough space to reallocate with offset and length
    // in commit, but at this stage we do not know what offset and length will be.
    auto mem = _outputWriter.allocate(_outputWriter.maxSize());

    if (mem.size() != _outputWriter.maxSize())
    {
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

    _buffer               = mem.first(size);
    _lastAllocationFailed = false;
    return _buffer;
}

void LegacyTxAdapter::commit()
{
    if (_buffer.empty())
    {
        return;
    }

    auto const bufferSize = _buffer.size();
    auto buffer           = _buffer;
    _buffer               = {};

    if (bufferSize < 2 * sizeof(::etl::be_uint32_t))
    {
        logger::Logger::error(
            logger::ROUTING,
            "Buffer size %u is too small to contain message ID and length",
            static_cast<uint32_t>(bufferSize));

        ++_droppedPduCounter;
        _droppedByteCounter += static_cast<uint32_t>(bufferSize);

        return;
    }

    auto const messageId = buffer.take<::etl::be_uint32_t>();
    auto const* const messageIdPos
        = ::etl::find(_table.messageIds.begin(), _table.messageIds.end(), messageId);

    if (messageIdPos == _table.messageIds.end())
    {
        _errorHandler(ErrorHandler::StatusCode::UNKNOWN_MESSAGE_ID);

        logger::Logger::error(
            logger::ROUTING,
            "Attempting to output unknown message %u",
            static_cast<uint32_t>(messageId));

        ++_unknownMessagePdus;

        ++_droppedPduCounter;
        _droppedByteCounter += static_cast<uint32_t>(bufferSize);

        return;
    }

    size_t const index
        = static_cast<size_t>(::etl::distance(_table.messageIds.begin(), messageIdPos));
    uint32_t const messageLength = _table.messageLengths[index];
    uint32_t const pduOffset     = _table.pduOffsets[index];

    // length
    (void)buffer.take<::etl::be_uint32_t const>();

    // space for ID + size + payload of message
    size_t const messageBufferSize = messageLength + 2 * sizeof(::etl::be_uint32_t);
    auto out                       = _outputWriter.allocate(messageBufferSize);
    // Since _outputWriter.maxSize() bytes are available in the queue, the allocation fails only if
    // messageBufferSize is greater than that, i.e., it is an invalid entry in the table.
    if (out.size() != messageBufferSize)
    {
        // The last memory allocation did not fail because _buffer was not empty.
        _errorHandler(ErrorHandler::StatusCode::MEM_ALLOCATION_FAILURE);
        _lastAllocationFailed = true;

        logger::Logger::error(
            logger::ROUTING,
            "Failed to allocate output message (ID=%u,length=%u)",
            static_cast<uint32_t>(messageId),
            static_cast<uint32_t>(messageLength));

        ++_failedMemAllocPdus;

        ++_droppedPduCounter;
        _droppedByteCounter += static_cast<uint32_t>(messageBufferSize);

        return;
    }

    if (pduOffset >= messageLength)
    {
        logger::Logger::error(
            logger::ROUTING,
            "Invalid PDU offset (ID=%u,msg-length=%u,offset=%u,buf-size=%u)",
            static_cast<uint32_t>(messageId),
            static_cast<uint32_t>(messageLength),
            static_cast<uint32_t>(pduOffset),
            static_cast<uint32_t>(buffer.size()));

        ++_failedMemMovePdus;

        ++_droppedPduCounter;
        _droppedByteCounter += static_cast<uint32_t>(messageBufferSize);

        return;
    }

    out.take<::etl::be_uint32_t>() = messageId;
    out.take<::etl::be_uint32_t>() = static_cast<uint32_t>(messageLength);

    auto const payloadLength
        = ::etl::min(out.size() - static_cast<size_t>(pduOffset), buffer.size());
    if (pduOffset != 0)
    {
        // The buffer points to the same memory, so first move it with memmove which handles
        // overlap, then fill the the gap with 0xFF.
        (void)::etl::mem_move(buffer.begin(), payloadLength, out.subspan(pduOffset).begin());
        (void)::etl::mem_set(out.first(pduOffset).begin(), pduOffset, static_cast<uint8_t>(0xFF));
    }

    // This needs to be done whether there is an offset or not.
    out.advance(pduOffset + payloadLength);
    (void)::etl::mem_set(out.begin(), out.size(), static_cast<uint8_t>(0xFF));

    _outputWriter.commit();

    ++_pduCounter;
    _byteCounter += static_cast<uint32_t>(messageBufferSize);

    return;
}

void LegacyTxAdapter::flush() { _outputWriter.flush(); }

// NOLINTEND(cppcoreguidelines-pro-type-vararg)

} // namespace routing
