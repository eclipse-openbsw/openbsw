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
#include "routing/RxAdapter.h"
#include "routing/RxAdapterTable.h"
#include "routing/util.h"

#include <etl/algorithm.h>
#include <etl/array.h>
#include <etl/iterator.h>
#include <etl/span.h>
#include <io/IReader.h>

#include <platform/estdint.h>

namespace routing
{
/**
 * Extracts the PDUs contained in the messages read from inputReader, adapting the IReader
 * interface, according to the provided RxAdapterTable.
 *
 * [TPARAMS_BEGIN]
 * \tparam MAX_ELEMENT_SIZE The maximum size of an extracted PDU.
 * [TPARAMS_END]
 */
template<size_t MAX_ELEMENT_SIZE>
class PduTransportRxAdapter : public RxAdapter
{
public:
    // [PUBLIC_API_BEGIN]
    /**
     * Construct a PduTransportRxAdapter with a pre-filled RxAdapterTable.
     */
    PduTransportRxAdapter(
        ::io::IReader& inputReader, RxAdapterTable const& table, ErrorHandler errorHandler);

    /**
     * Return the MAX_ELEMENT_SIZE
     */
    size_t maxSize() const override;

    ::etl::span<uint8_t> peek() const override;

    // The discarded PDU and byte counters store the amount of data in incoming messages out of
    // which not a single PDU was extracted.
    StatCounter::Type invalidMessageLengthPdus() const { return _invalidMessageLengthPdus; }

    // [PUBLIC_API_END]

protected:
    ErrorHandler::StatusCode lookup(::etl::span<uint8_t const> message) const override;

private:
    ErrorHandler::StatusCode processMessage(::etl::span<uint8_t const> frame) const;

    ::etl::array<uint8_t, MAX_ELEMENT_SIZE> _pduBuffer;

    mutable ::etl::span<uint8_t const> _currentFrame;
    mutable size_t _currentMessageSize;
    mutable uint32_t _invalidMessageLengthPdus;
};

template<size_t MAX_ELEMENT_SIZE>
PduTransportRxAdapter<MAX_ELEMENT_SIZE>::PduTransportRxAdapter(
    ::io::IReader& inputReader, RxAdapterTable const& table, ErrorHandler const errorHandler)
: RxAdapter(inputReader, _pduBuffer, table, errorHandler)
, _currentFrame()
, _currentMessageSize()
, _invalidMessageLengthPdus()
{}

template<size_t MAX_ELEMENT_SIZE>
size_t PduTransportRxAdapter<MAX_ELEMENT_SIZE>::maxSize() const
{
    return MAX_ELEMENT_SIZE;
}

template<size_t MAX_ELEMENT_SIZE>
ErrorHandler::StatusCode
PduTransportRxAdapter<MAX_ELEMENT_SIZE>::processMessage(::etl::span<uint8_t const> frame) const
{
    if (frame.size() < MESSAGE_HEADER_SIZE)
    {
        _currentMessageSize = frame.size();

        ++_invalidHeaderSizePdus;

        return ErrorHandler::StatusCode::INVALID_MESSAGE_HEADER_SIZE;
    }

    auto header = frame.first(MESSAGE_HEADER_SIZE);

    (void)header.take<::etl::be_uint32_t const>(); // message ID
    auto const parsedPayloadLength = header.take<::etl::be_uint32_t const>();
    auto const messageSize         = MESSAGE_HEADER_SIZE + parsedPayloadLength;
    if (messageSize > frame.size())
    {
        _currentMessageSize = frame.size();

        ++_invalidParsedPayloadLengthPdus;

        return ErrorHandler::StatusCode::INVALID_PARSED_LENGTH;
    }
    _currentMessageSize = messageSize;

    auto const message = frame.take<uint8_t const>(_currentMessageSize);
    return lookup(message);
}

template<size_t MAX_ELEMENT_SIZE>
::etl::span<uint8_t> PduTransportRxAdapter<MAX_ELEMENT_SIZE>::peek() const
{
    // Return the last PDU extracted.
    if (!_currentPdu.empty())
    {
        return _currentPdu;
    }

    if (_currentFrame.empty())
    {
        _currentFrame = _inputReader.peek();
    }

    while (!_currentFrame.empty())
    {
        bool const isNewMessage = _currentPduIndex == 0;
        if (isNewMessage)
        {
            auto const statusCode = processMessage(_currentFrame);

            ++_pduCounter;
            _byteCounter += static_cast<uint32_t>(_currentMessageSize);

            if (statusCode != ErrorHandler::StatusCode::OK)
            {
                _errorHandler(statusCode, _currentFrame.first(_currentMessageSize));

                ++_discardedPduCounter;
                _discardedByteCounter += static_cast<uint32_t>(_currentMessageSize);

                _currentFrame.advance(_currentMessageSize);

                if (_currentFrame.empty())
                {
                    _inputReader.release();
                    _currentFrame = _inputReader.peek();
                }

                continue;
            }
        }

        bool noPduExtracted     = true;
        auto const numberOfPdus = _table.pduLengthsOffsets[_currentMessageIndex + 1]
                                  - _table.pduLengthsOffsets[_currentMessageIndex];
        auto const message = _currentFrame.first(_currentMessageSize);
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
            _currentPduIndex = 0;
            _currentFrame.advance(_currentMessageSize);

            if (_currentFrame.empty())
            {
                _inputReader.release();
                _currentFrame = _inputReader.peek();
            }
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
                _discardedByteCounter += static_cast<uint32_t>(_currentMessageSize);
            }

            continue;
        }

        return _currentPdu;
    }
    return {};
}

template<size_t MAX_ELEMENT_SIZE>
ErrorHandler::StatusCode
PduTransportRxAdapter<MAX_ELEMENT_SIZE>::lookup(::etl::span<uint8_t const> message) const
{
    auto const messageId = message.take<::etl::be_uint32_t const>();
    auto const pos = ::etl::find(_table.messageIds.begin(), _table.messageIds.end(), messageId);
    if (pos == _table.messageIds.end())
    {
        ++_unknownMessagePdus;

        return ErrorHandler::StatusCode::UNKNOWN_MESSAGE_ID;
    }

    uint32_t const parsedPayloadLength = message.take<::etl::be_uint32_t const>();

    auto const index = static_cast<size_t>(::etl::distance(_table.messageIds.begin(), pos));
    uint32_t const messageLength = (!_table.messageLengths.empty())
                                       ? static_cast<uint32_t>(_table.messageLengths[index])
                                       : 0U;

    if ((messageLength != 0) && (parsedPayloadLength != messageLength))
    {
        ++_invalidMessageLengthPdus;

        return ErrorHandler::StatusCode::INVALID_MESSAGE_LENGTH;
    }

    _currentMessageIndex = index;

    return ErrorHandler::StatusCode::OK;
}

} // namespace routing
