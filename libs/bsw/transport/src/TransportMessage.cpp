// Copyright 2024 Accenture.

#include "transport/TransportMessage.h"

#include "transport/IDataProgressListener.h"
#include "transport/TransportLogger.h"

#include <estd/assert.h>
#include <estd/memory.h>

#include <cstring>

namespace transport
{
using ::util::logger::_CRITICAL;
using ::util::logger::Logger;
using ::util::logger::TRANSPORT;

TransportMessage::TransportMessage()
: fpDataProgressListener(nullptr)
, fBuffer()
, fSourceId(INVALID_ADDRESS)
, fTargetId(INVALID_ADDRESS)
, fPayloadLength(0U)
, fValidBytes(0U)
{}

TransportMessage::TransportMessage(uint8_t* const buffer, uint32_t const bufferLength)
: fpDataProgressListener(nullptr)
, fBuffer(::estd::slice<uint8_t>::from_pointer(buffer, bufferLength))
, fSourceId(INVALID_ADDRESS)
, fTargetId(INVALID_ADDRESS)
, fPayloadLength(0U)
, fValidBytes(0U)
{}

void TransportMessage::init(uint8_t* const buffer, uint32_t const bufferLength)
{
    if ((buffer == nullptr) && (bufferLength > 0U))
    {
        Logger::critical(
            TRANSPORT,
            "TransportMessage::init(): buffer is NULL but bufferLength is %d!",
            bufferLength);
        estd_assert(false);
    }
    fpDataProgressListener = nullptr;
    fBuffer                = ::estd::slice<uint8_t>::from_pointer(buffer, bufferLength);
    fValidBytes            = 0U;
    if (fBuffer.size() != 0U)
    {
        setPayloadLength(0U);
    }
}

void TransportMessage::setServiceId(uint8_t const theServiceId)
{
    if (fBuffer.size() == 0U)
    {
        Logger::critical(TRANSPORT, "TransportMessage::setServiceId(): fpBuffer is NULL!");
        estd_assert(false);
    }
    fBuffer[SERVICE_ID_INDEX] = theServiceId;
    // to be consistent with append, valid bytes must be increased here!
    if (0U == fValidBytes)
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
        estd_assert(false);
    }
    fPayloadLength = length;
}

TransportMessage::ErrorCode
TransportMessage::append(uint8_t const* const data, uint16_t const length)
{
    if ((fValidBytes + length) > getMaxPayloadLength())
    {
        return ErrorCode::TP_MSG_LENGTH_EXCEEDED;
    }

    (void)::estd::memory::copy(
        fBuffer.offset(fValidBytes), ::estd::slice<uint8_t const>::from_pointer(data, length));
    (void)increaseValidBytes(length);
    return ErrorCode::TP_MSG_OK;
}

TransportMessage::ErrorCode TransportMessage::append(uint8_t const data)
{
    if ((fValidBytes + 1U) > getMaxPayloadLength())
    {
        return ErrorCode::TP_MSG_LENGTH_EXCEEDED;
    }
    fBuffer[static_cast<size_t>(fValidBytes)] = data;
    (void)increaseValidBytes(1U);
    return ErrorCode::TP_MSG_OK;
}

TransportMessage::ErrorCode TransportMessage::increaseValidBytes(uint16_t const n)
{
    if ((fValidBytes + n) > getMaxPayloadLength())
    {
        // this is an overflow, we only add as much as possible
        uint32_t const numberOfNewValidBytes
            = static_cast<uint32_t>(getMaxPayloadLength()) - static_cast<uint32_t>(fValidBytes);
        fValidBytes = getMaxPayloadLength();
        notifyDataProgressListener(numberOfNewValidBytes);
        return ErrorCode::TP_MSG_LENGTH_EXCEEDED;
    }

    fValidBytes += n;
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
    if (fValidBytes != rhs.fValidBytes)
    {
        return false;
    }
    if ((getTargetId() != rhs.getTargetId()) || (getSourceId() != rhs.getSourceId()))
    {
        return false;
    }

    return 0 == ::memcmp(rhs.fBuffer.data(), fBuffer.data(), fValidBytes);
}

void TransportMessage::setDataProgressListener(IDataProgressListener& listener)
{
    fpDataProgressListener = &listener;
}

bool TransportMessage::isDataProgressListener(IDataProgressListener const& listener) const
{
    return fpDataProgressListener == &listener;
}

void TransportMessage::removeDataProgressListener() { fpDataProgressListener = nullptr; }

void TransportMessage::notifyDataProgressListener(uint32_t const numberOfNewValidBytes)
{
    if (fpDataProgressListener != nullptr)
    {
        fpDataProgressListener->dataProgressed(*this, numberOfNewValidBytes);
    }
}

} // namespace transport
