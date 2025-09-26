// Copyright 2024 Accenture.

#include "can/canframes/CANFrame.h"

#include "can/canframes/CanId.h"

#if (!defined(BOOTLOADER)) && (!defined(BOOTLOADER_UPDATER))
#include "can/CanLogger.h"
using ::util::logger::Logger;
#endif

#include <etl/error_handler.h>

namespace can
{
// define const variables for GCC
// #ifdef __GNUC__
uint8_t const CANFrame::SENDER_MASK;
uint8_t const CANFrame::CAN_OVERHEAD_BITS;
uint8_t const CANFrame::MAX_FRAME_LENGTH;
uint32_t const CANFrame::MAX_FRAME_ID;
uint32_t const CANFrame::MAX_FRAME_ID_EXTENDED;

// #endif

CANFrame::CANFrame() : _id(0U), _timestamp(0U), _payloadLength(0U)
{
    (void)memset(_payload, 0, sizeof(_payload));
}

CANFrame::CANFrame(uint32_t const id) : _id(id), _timestamp(0U), _payloadLength(0U)
{
    (void)memset(_payload, 0xFF, sizeof(_payload));
}

CANFrame::CANFrame(CANFrame const& frame)
: _id(frame._id), _timestamp(frame._timestamp), _payloadLength(frame._payloadLength)
{
    (void)memcpy(_payload, frame._payload, static_cast<size_t>(frame._payloadLength));
}

CANFrame::CANFrame(uint32_t const id, uint8_t const* const payload, uint8_t const length)
: _id(id), _timestamp(0U), _payloadLength(length)
{
    ETL_ASSERT(
        length <= MAX_FRAME_LENGTH,
        ETL_ERROR_GENERIC("can frame length must be smaller than maximum length"));

    (void)memcpy(_payload, payload, static_cast<size_t>(length));
}

CANFrame::CANFrame(
    uint32_t const rawId,
    uint8_t const* const payload,
    uint8_t const length,
    bool const isExtendedId)
: _id(CanId::id(rawId, isExtendedId)), _timestamp(0U), _payloadLength(length)
{
    ETL_ASSERT(
        rawId <= MAX_FRAME_ID_EXTENDED,
        ETL_ERROR_GENERIC("can frame id must be smaller than the maximum allowed one"));

    ETL_ASSERT(
        length <= MAX_FRAME_LENGTH,
        ETL_ERROR_GENERIC("can frame length must be smaller than maximum length"));

    (void)memcpy(_payload, payload, static_cast<size_t>(length));
}

CANFrame& CANFrame::operator=(CANFrame const& canFrame)
{
    if (&canFrame != this)
    {
        _id            = canFrame._id;
        _timestamp     = canFrame._timestamp;
        _payloadLength = canFrame._payloadLength;
        (void)memcpy(_payload, canFrame._payload, static_cast<size_t>(canFrame._payloadLength));
    }
    return *this;
}

void CANFrame::setPayloadLength(uint8_t const length)
{
    // only accept a payload length less or equal the maximum
    if (length <= MAX_FRAME_LENGTH)
    {
        _payloadLength = length;
    }
}

bool operator==(CANFrame const& frame1, CANFrame const& frame2)
{
    if (frame1._id != frame2._id)
    {
        return false;
    }
    if (frame1._payloadLength != frame2._payloadLength)
    {
        return false;
    }
    for (uint8_t i = 0U; i < frame1._payloadLength; ++i)
    {
        if (frame1._payload[i] != frame2._payload[i])
        {
            return false;
        }
    }
    return true;
}

} // namespace can
