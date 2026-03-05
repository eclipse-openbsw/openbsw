// Copyright 2024 Accenture.

#include "uds/connection/ResponseMock.h"
#include <etl/unaligned_type.h>

class PositiveResponse;

namespace uds
{

size_t PositiveResponse::appendData(uint8_t const data[], size_t length)
{
    if (PositiveResponseMockHelper::instance().isStub())
    {
        return length;
    }
    return PositiveResponseMockHelper::instance()
        .appendData(data, length);
}

bool PositiveResponse::appendUint8(uint8_t const data) { return appendData(&data, 1U) == 1U; }

bool PositiveResponse::appendUint16(uint16_t const data)
{
    auto const v = ::etl::be_uint16_t(data);
    return appendData(v.data(), 2) == 2;
}

bool PositiveResponse::appendUint24(uint32_t const data)
{
    auto const v = ::etl::be_uint32_t(data);
    return appendData(v.data() + 1, 3) == 3;
}

bool PositiveResponse::appendUint32(uint32_t const data)
{
    auto const v = ::etl::be_uint32_t(data);
    return appendData(v.data(), 4) == 4;
}

uint8_t* PositiveResponse::getData()
{
    if (PositiveResponseMockHelper::instance().isStub())
    {
        static uint8_t rspbuf[256];
        return rspbuf;
    }
    return PositiveResponseMockHelper::instance()
        .getData();
}

size_t PositiveResponse::getLength() const
{
    if (PositiveResponseMockHelper::instance().isStub())
    {
        return PositiveResponseMockHelper::instance().getLength();
    }
    return 0;
}

void PositiveResponse::init(uint8_t buffer[], size_t maximumLength) { fIsOverflow = false; }
} // namespace uds
