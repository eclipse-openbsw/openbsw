// Copyright 2024 Accenture.

#include "uds/connection/PositiveResponse.h"

#include <estd/assert.h>
#include <estd/big_endian.h>
#include <estd/memory.h>

#include <cstring>

namespace uds
{
void PositiveResponse::init(uint8_t* const buffer, size_t const maximumLength)
{
    fIsOverflow     = false;
    // Store the original pointer and length in fOriginalBuffer
    fOriginalBuffer = ::estd::slice<uint8_t>::from_pointer(buffer, maximumLength);
    reset();
}

void PositiveResponse::reset()
{
    // Initialize fBuffer as the current buffer to work with. fBuffer will always
    // point to the current write/append destination.
    fBuffer = fOriginalBuffer;
}

bool PositiveResponse::appendUint8(uint8_t const data) { return appendData(&data, 1U) == 1U; }

bool PositiveResponse::appendUint16(uint16_t const data)
{
    auto const v = ::estd::be_uint16_t::make(data);
    return appendData(v.bytes, 2) == 2;
}

bool PositiveResponse::appendUint24(uint32_t const data)
{
    auto const v = ::estd::be_uint24_t::make(data);
    return appendData(v.bytes, 3) == 3;
}

bool PositiveResponse::appendUint32(uint32_t const data)
{
    auto const v = ::estd::be_uint32_t::make(data);
    return appendData(v.bytes, 4) == 4;
}

size_t PositiveResponse::appendData(uint8_t const* const data, size_t const length)
{
    auto const sourceData = ::estd::slice<uint8_t const>::from_pointer(data, length);
    if (fBuffer.size() < sourceData.size())
    {
        fIsOverflow = true;
        return 0;
    }
    auto const result = ::estd::memory::copy(fBuffer, sourceData);
    fBuffer.advance(result.size());
    return static_cast<uint16_t>(result.size());
}

size_t PositiveResponse::getMaximumLength() const { return fOriginalBuffer.size(); }

size_t PositiveResponse::getLength() const
{
    // Length of the response is maximum length minus current write position.
    return getMaximumLength() - fBuffer.size();
}

uint8_t* PositiveResponse::getData() { return fBuffer.data(); }

size_t PositiveResponse::getAvailableDataLength() const { return fBuffer.size(); }

size_t PositiveResponse::increaseDataLength(size_t const length)
{
    if (getAvailableDataLength() < length)
    {
        // The bounds of the buffer have been overwritten! Unfortunately we cannot prevent it
        // because we have exposed the pointer to the buffer by having a public getData() method!
        // The system is now in potentially in an unpredictable state. Therefore, asserting here is
        // only fair!
        estd_assert(false);
        return 0U;
    }
    fBuffer.advance(length);
    return getLength();
}

} // namespace uds
