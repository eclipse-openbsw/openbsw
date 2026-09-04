/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SomeIpParser.h"

#include "someip/ISomeIpSerializable.h"
#include "someip/PrimitiveTypes.h"

namespace someip
{
void skip(SomeIpParser& parser, uint16_t const tag)
{
    uint32_t const wireType = (static_cast<uint32_t>(tag) >> 12U);
    switch (wireType)
    {
        case 0U:
        {
            parser.skip(sizeof(uint8_t));
            break;
        }
        case 1U:
        {
            parser.skip(sizeof(uint16_t));
            break;
        }
        case 2U:
        {
            parser.skip(sizeof(uint32_t));
            break;
        }
        case 3U:
        {
            parser.skip(sizeof(uint64_t));
            break;
        }
        default:
        {
            uint32_t length = 0U;
            parser >> length;
            parser.skip(static_cast<size_t>(length));
            break;
        }
    }
}

SomeIpParser::SomeIpParser(::etl::span<uint8_t const> const buffer)
: _buffer(buffer), _currentPos(0U)
{}

void SomeIpParser::operator>>(float_t& item)
{
    if (!isGood())
    {
        return;
    }
    if (hasSpace(4))
    {
        uint32_t const bits = ::someip::endian::read_be<uint32_t>(&_buffer[_currentPos]);
        item                = ::someip::internal::unpackIEEE754<float_t, uint32_t, 8U>(bits);
        _currentPos += 4U;
    }
}

void SomeIpParser::operator>>(double_t& item)
{
    if (!isGood())
    {
        return;
    }
    if (hasSpace(8))
    {
        if (hasSpace(sizeof(item)))
        {
            uint64_t const bits = ::someip::endian::read_be<uint64_t>(&_buffer[_currentPos]);
            item                = ::someip::internal::unpackIEEE754<double_t, uint64_t, 11U>(bits);
            _currentPos += 8U;
        }
    }
}

void SomeIpParser::operator>>(ISomeIpSerializable& item) { item.parseFromArray(*this); }

void SomeIpParser::skip(size_t const bytes)
{
    if (hasSpace(bytes))
    {
        _currentPos += bytes;
    }
}

uint32_t SomeIpParser::readTypeFieldSize()
{
    _bigEndian             = true;
    uint8_t const typeSize = _typeFieldSize;

    if (typeSize == 1U)
    {
        uint8_t fieldSize = 0U;
        operator>>(fieldSize);
        return static_cast<uint32_t>(fieldSize);
    }
    if (typeSize == 2U)
    {
        uint16_t fieldSize = 0U;
        operator>>(fieldSize);
        return static_cast<uint32_t>(fieldSize);
    }

    // default is uint32_t field size
    uint32_t fieldSize = 0U;
    operator>>(fieldSize);
    return fieldSize;
}

} // namespace someip
