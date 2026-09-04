/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "someip/SomeIpParser.h"
#include "someip/SomeIpSerializer.h"

#include "someip/endian_helpers.h"
#include <etl/span.h>

namespace someip
{
template<class LENGTH_TYPE>
class LengthSerializerHelper
{
public:
    explicit LengthSerializerHelper(SomeIpSerializer& serializer)
    : serializer(serializer)
    , lengthBuffer(serializer.skipBytes(sizeof(LENGTH_TYPE)))
    , writingStartPos(serializer.getCurrentPosition())

    {}

    ~LengthSerializerHelper()
    {
        if (lengthBuffer.size() == sizeof(LENGTH_TYPE))
        {
            size_t const totalBytesWritten = serializer.getCurrentPosition() - writingStartPos;
            ::someip::endian::write_be<LENGTH_TYPE>(
                lengthBuffer.data(), static_cast<LENGTH_TYPE>(totalBytesWritten));
        }
    }

private:
    SomeIpSerializer& serializer;
    ::etl::span<uint8_t> lengthBuffer;
    size_t writingStartPos;
};

template<class LENGTH_TYPE>
class LengthParserHelper
{
public:
    static uint32_t parseLength(SomeIpParser& parser);
};

template<class LENGTH_TYPE>
uint32_t LengthParserHelper<LENGTH_TYPE>::parseLength(SomeIpParser& parser)
{
    if (!parser.isGood())
    {
        return 0U;
    }

    LENGTH_TYPE numBytes = 0U;

    // for now length's are always written in BigEndian
    parser.bigEndian();
    parser >> numBytes;

    if (parser.bytesAvailable() < numBytes)
    {
        parser.setFailure();
        return 0U;
    }

    return static_cast<uint32_t>(numBytes);
}

} // namespace someip
