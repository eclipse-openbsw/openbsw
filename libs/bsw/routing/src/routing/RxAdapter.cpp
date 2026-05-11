/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/RxAdapter.h"

#include "routing/Logger.h"

#include <etl/algorithm.h>
#include <etl/iterator.h>
#include <etl/span.h>
#include <etl/unaligned_type.h>
#include <util/logger/Logger.h>

namespace routing
{
RxAdapter::RxAdapter(
    ::io::IReader& inputReader,
    ::etl::span<uint8_t> const buffer,
    RxAdapterTable const& table,
    ErrorHandler const errorHandler)
: _inputReader(inputReader)
, _buffer(buffer)
, _table(table)
, _errorHandler(errorHandler)
, _currentMessageIndex(0U)
, _currentPduIndex(0U)
, _currentPdu()
, _pduCounter()
, _byteCounter()
, _invalidHeaderSizePdus()
, _unknownMessagePdus()
, _invalidParsedPayloadLengthPdus()
, _invalidPayloadLengthPdus()
, _discardedPduCounter()
, _discardedByteCounter()
{}

RxAdapter::RxAdapter(
    ::io::IReader& inputReader,
    ::etl::span<uint8_t> const buffer,
    ::etl::span<uint8_t const> const config,
    ErrorHandler const errorHandler)
: RxAdapter(inputReader, buffer, RxAdapterTable(), errorHandler)
{
    if (!load(config, _table))
    {
        ::util::logger::Logger::error(
            ::util::logger::ROUTING, "Failed to load RX adapter table from config");
    }
}

size_t RxAdapter::maxSize() const { return _buffer.size(); }

ErrorHandler::StatusCode RxAdapter::lookup(::etl::span<uint8_t const> message) const
{
    if (message.size() < MESSAGE_HEADER_SIZE)
    {
        ++_invalidHeaderSizePdus;

        return ErrorHandler::StatusCode::INVALID_MESSAGE_HEADER_SIZE;
    }

    auto const messageId           = message.take<::etl::be_uint32_t const>();
    auto const parsedPayloadLength = message.take<::etl::be_uint32_t const>();

    if (parsedPayloadLength > message.size())
    {
        ++_invalidParsedPayloadLengthPdus;

        return ErrorHandler::StatusCode::INVALID_PARSED_LENGTH;
    }

    auto const pos = ::etl::find(_table.messageIds.begin(), _table.messageIds.end(), messageId);
    if (pos == _table.messageIds.end())
    {
        ++_unknownMessagePdus;

        return ErrorHandler::StatusCode::UNKNOWN_MESSAGE_ID;
    }

    _currentMessageIndex = static_cast<size_t>(::etl::distance(_table.messageIds.begin(), pos));

    return ErrorHandler::StatusCode::OK;
}

// Return whether a PDU was extracted or not
bool RxAdapter::extract(
    ::etl::span<uint8_t const> message,
    size_t const channelSpecificPduId,
    ::etl::span<uint8_t>& pdu) const
{
    auto messagePayload    = message.subspan(MESSAGE_HEADER_SIZE);
    size_t const pduOffset = _table.pduOffsets[channelSpecificPduId];
    size_t const pduLength = _table.pduLengths[channelSpecificPduId];

    if ((pduOffset + pduLength) > messagePayload.size())
    {
        return false;
    }

    messagePayload.advance(pduOffset);
    auto const payload = messagePayload.take<uint8_t const>(pduLength);

    if ((MESSAGE_HEADER_SIZE + payload.size()) > pdu.size())
    {
        return false;
    }

    pdu                            = pdu.first(MESSAGE_HEADER_SIZE + payload.size());
    auto mem                       = pdu;
    mem.take<::etl::be_uint32_t>() = static_cast<uint32_t>(_table.firstId + channelSpecificPduId);
    mem.take<::etl::be_uint32_t>() = static_cast<uint32_t>(payload.size());

    return ::etl::copy(payload, mem);
}

::etl::span<uint8_t> RxAdapter::peek() const
{
    // Return the last PDU extracted.
    if (!_currentPdu.empty())
    {
        return _currentPdu;
    }

    ::etl::span<uint8_t const> message = _inputReader.peek();
    while (!message.empty())
    {
        bool const isNewMessage = _currentPduIndex == 0;
        if (isNewMessage)
        {
            ++_pduCounter;
            _byteCounter += static_cast<uint32_t>(message.size());

            auto const statusCode = lookup(message);
            if (statusCode != ErrorHandler::StatusCode::OK)
            {
                _errorHandler(statusCode, message);

                ++_discardedPduCounter;
                _discardedByteCounter += static_cast<uint32_t>(message.size());

                _inputReader.release();
                message = _inputReader.peek();

                continue;
            }
        }

        bool noPduExtracted     = true;
        auto const numberOfPdus = _table.pduLengthsOffsets[_currentMessageIndex + 1]
                                  - _table.pduLengthsOffsets[_currentMessageIndex];
        while (noPduExtracted && (_currentPduIndex < numberOfPdus))
        {
            auto const channelSpecificPduId
                = _table.pduLengthsOffsets[_currentMessageIndex] + _currentPduIndex;
            auto pdu = _buffer;
            if (extract(message, channelSpecificPduId, pdu))
            {
                _currentPdu    = pdu;
                noPduExtracted = false;
            }

            ++_currentPduIndex;
        }

        if (_currentPduIndex == numberOfPdus)
        {
            _inputReader.release();
            _currentPduIndex = 0;
        }

        if (noPduExtracted)
        {
            if (isNewMessage)
            {
                // Although this message was expected, no PDU was extracted from it because all the
                // expected PDUs were too big
                _errorHandler(ErrorHandler::StatusCode::INVALID_PAYLOAD_LENGTH, message);

                ++_invalidPayloadLengthPdus;

                ++_discardedPduCounter;
                _discardedByteCounter += static_cast<uint32_t>(message.size());
            }
            message = _inputReader.peek();

            continue;
        }

        return _currentPdu;
    }
    return {};
}

void RxAdapter::release() { _currentPdu = {}; }

} // namespace routing
