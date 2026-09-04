/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SomeIpSerializer.h"

#include "someip/PrimitiveTypes.h"

#include <etl/algorithm.h>

namespace someip
{
SomeIpSerializer::SomeIpSerializer(::etl::span<uint8_t> const buffer)
: _buffer(buffer), _currentPos(0U)
{}

void SomeIpSerializer::operator<<(float_t const item)
{
    if (!isGood())
    {
        return;
    }
    if (hasSpace(4))
    {
        // switch to big endian temporarily
        uint32_t const tmp = ::someip::internal::packIEEE754<uint32_t, float_t, 8U>(item);
        ::someip::endian::write_be<uint32_t>(&_buffer[_currentPos], tmp);
        _currentPos += 4U;
    }
}

void SomeIpSerializer::operator<<(double_t const item)
{
    if (!isGood())
    {
        return;
    }
    if (hasSpace(8))
    {
        uint64_t const tmp = ::someip::internal::packIEEE754<uint64_t, double_t, 11U>(item);
        ::someip::endian::write_be<uint64_t>(&_buffer[_currentPos], tmp);
        _currentPos += 8U;
    }
}

void SomeIpSerializer::operator<<(ISomeIpSerializable const& item) { item.serializeToArray(*this); }

void SomeIpSerializer::operator<<(::etl::span<uint8_t const> const data)
{
    if (!isGood())
    {
        return;
    }
    if (hasSpace(data.size()))
    {
        auto dest = _buffer.subspan(_currentPos);
        etl::copy(data.begin(), data.end(), dest.begin());
        _currentPos += data.size();
    }
}

void SomeIpSerializer::writeTypeFieldSize(uint32_t const fieldType)
{
    bigEndian();
    uint8_t const typeSize = getTypeFieldSize();

    if (typeSize == 1U)
    {
        operator<<(static_cast<uint8_t>(fieldType));
        return;
    }
    if (typeSize == 2U)
    {
        operator<<(static_cast<uint16_t>(fieldType));
        return;
    }

    // default is uint32_t field size
    operator<<(static_cast<uint32_t>(fieldType));
}

} // namespace someip
