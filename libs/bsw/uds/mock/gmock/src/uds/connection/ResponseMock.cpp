// Copyright 2024 Accenture.

#include "uds/connection/ResponseMock.h"

#include "uds/connection/IncomingDiagConnectionMock.h"

#include <etl/unaligned_type.h>

namespace uds
{

size_t PositiveResponse::appendData(uint8_t const data[], size_t length)
{
    if (PositiveResponseMockHelper::instance(IncomingDiagConnectionMockHelper::instance()).isStub())
    {
        return length;
    }
    return PositiveResponseMockHelper::instance(IncomingDiagConnectionMockHelper::instance())
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
    if (PositiveResponseMockHelper::instance(IncomingDiagConnectionMockHelper::instance()).isStub())
    {
        static uint8_t rspbuf[256];
        return rspbuf;
    }
    return PositiveResponseMockHelper::instance(IncomingDiagConnectionMockHelper::instance())
        .getData();
}

::uds::ErrorCode PositiveResponse::send(AbstractDiagJob& sender)
{
    if (PositiveResponseMockHelper::instance(IncomingDiagConnectionMockHelper::instance()).isStub())
    {
        return ::uds::ErrorCode::OK;
    }
    return PositiveResponseMockHelper::instance(IncomingDiagConnectionMockHelper::instance())
        .send(sender);
}

void PositiveResponse::init(uint8_t buffer[], size_t maximumLength) { fIsOverflow = false; }
} // namespace uds
