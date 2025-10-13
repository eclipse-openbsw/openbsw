// Copyright 2024 Accenture.

#include "transport/TransportMessage.h"

#include "transport/IDataProgressListener.h"
#include "transport/TransportLogger.h"

#include <etl/error_handler.h>

#include <cstring>

namespace transport
{
using ::util::logger::_CRITICAL;
using ::util::logger::Logger;
using ::util::logger::TRANSPORT;

TransportMessage::TransportMessage()
: _dataProgressListener(nullptr)
, _buffer()
, _sourceAddress(INVALID_ADDRESS)
, _targetAddress(INVALID_ADDRESS)
, _payloadLength(0U)
, _validBytes(0U)
{}

TransportMessage::TransportMessage(uint8_t* const buffer, uint32_t const bufferLength)
: _dataProgressListener(nullptr)
, _buffer(::etl::span<uint8_t>(buffer, bufferLength))
, _sourceAddress(INVALID_ADDRESS)
, _targetAddress(INVALID_ADDRESS)
, _payloadLength(0U)
, _validBytes(0U)
{}

void TransportMessage::init(uint8_t* const buffer, uint32_t const bufferLength)
{
    if ((buffer == nullptr) && (bufferLength > 0U))
    {
        Logger::critical(
            TRANSPORT,
            "TransportMessage::init(): buffer is NULL but bufferLength is %d!",
            bufferLength);
        ETL_ASSERT_FAIL(ETL_ERROR_GENERIC("buffer is null but buffer length is not zero"));
    }
    _dataProgressListener = nullptr;
    _buffer               = ::etl::span<uint8_t>(buffer, bufferLength);
    _validBytes           = 0U;
    if (_buffer.size() != 0U)
    {
        setPayloadLength(0U);
    }
}

void TransportMessage::setServiceId(uint8_t const theServiceId)
{
    if (_buffer.size() == 0U)
    {
        Logger::critical(TRANSPORT, "TransportMessage::setServiceId(): fpBuffer is NULL!");
        ETL_ASSERT_FAIL(ETL_ERROR_GENERIC("buffer size is zero"));
    }
    _buffer[SERVICE_ID_INDEX] = theServiceId;
    // to be consistent with append, valid bytes must be increased here!
    if (0U == _validBytes)
    {
        (void)increaseValidBytes(1U);
    }
}

void TransportMessage::setPayloadLength(uint16_t const length)
{
    if (length > getMaxPayloadLength())
    {
        Logger::critical(
            TRANSPORT,
            "TransportMessage::setPayloadLength(): length is too large (%d),"
            " maxLength is:%d!",
            length,
            getMaxPayloadLength());
        ETL_ASSERT_FAIL(ETL_ERROR_GENERIC("length is too large"));
    }
    _payloadLength = length;
}

TransportMessage::ErrorCode
TransportMessage::append(uint8_t const* const data, uint16_t const length)
{
    if ((_validBytes + length) > getMaxPayloadLength())
    {
        return ErrorCode::TP_MSG_LENGTH_EXCEEDED;
    }

    ::etl::span<uint8_t const> source(data, length);
    (void)::etl::copy(source, _buffer.subspan(_validBytes));
    (void)increaseValidBytes(length);
    return ErrorCode::TP_MSG_OK;
}

TransportMessage::ErrorCode TransportMessage::append(uint8_t const data)
{
    if ((_validBytes + 1U) > getMaxPayloadLength())
    {
        return ErrorCode::TP_MSG_LENGTH_EXCEEDED;
    }
    _buffer[static_cast<size_t>(_validBytes)] = data;
    (void)increaseValidBytes(1U);
    return ErrorCode::TP_MSG_OK;
}

TransportMessage::ErrorCode TransportMessage::increaseValidBytes(uint16_t const n)
{
    if ((_validBytes + n) > getMaxPayloadLength())
    {
        // this is an overflow, we only add as much as possible
        uint32_t const numberOfNewValidBytes
            = static_cast<uint32_t>(getMaxPayloadLength()) - static_cast<uint32_t>(_validBytes);
        _validBytes = getMaxPayloadLength();
        notifyDataProgressListener(numberOfNewValidBytes);
        return ErrorCode::TP_MSG_LENGTH_EXCEEDED;
    }

    _validBytes += n;
    notifyDataProgressListener(static_cast<uint32_t>(n));
    return ErrorCode::TP_MSG_OK;
}

bool TransportMessage::operator==(TransportMessage const& rhs) const
{
    if (getPayloadLength() != rhs.getPayloadLength())
    {
        return false;
    }
    // compare only valid bytes because a larger message may be used to receive
    // a small amount of data
    if (_validBytes != rhs._validBytes)
    {
        return false;
    }
    if ((getTargetId() != rhs.getTargetId()) || (getSourceId() != rhs.getSourceId()))
    {
        return false;
    }

    return 0 == ::memcmp(rhs._buffer.data(), _buffer.data(), _validBytes);
}

void TransportMessage::setDataProgressListener(IDataProgressListener& listener)
{
    _dataProgressListener = &listener;
}

bool TransportMessage::isDataProgressListener(IDataProgressListener const& listener) const
{
    return _dataProgressListener == &listener;
}

void TransportMessage::removeDataProgressListener() { _dataProgressListener = nullptr; }

void TransportMessage::notifyDataProgressListener(uint32_t const numberOfNewValidBytes)
{
    if (_dataProgressListener != nullptr)
    {
        _dataProgressListener->dataProgressed(*this, numberOfNewValidBytes);
    }
}

} // namespace transport
