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

#include "someip/ISomeIpSerializable.h"
#include "someip/SomeIpParser.h"
#include "someip/SomeIpSerializer.h"

#include <etl/span.h>
#include <cstdint>

namespace someip
{
class SomeIpSerializableBufferMock : public ISomeIpSerializable
{
public:
    explicit SomeIpSerializableBufferMock(::etl::span<uint8_t const> const* buffer)
    : _pSource(buffer), _pDrain(nullptr)
    {}

    explicit SomeIpSerializableBufferMock(::etl::span<uint8_t>* buffer)
    : _pSource(nullptr), _pDrain(buffer)
    {}

    void serializeToArray(SomeIpSerializer& serializer) const override
    {
        if (_pSource && _pSource->size() <= serializer.bytesAvailable())
        {
            for (auto c : *_pSource)
            {
                serializer << c;
            }
        }
        else
        {
            serializer.setFailure();
        }
    }

    void parseFromArray(SomeIpParser& parser) override
    {
        if (!_pDrain && _pDrain->size() >= parser.bytesAvailable())
        {
            for (size_t i = 0; i < parser.bytesAvailable(); ++i)
            {
                parser >> (*_pDrain)[i];
            }
        }
        else
        {
            parser.setFailure();
        }
    }

    uint32_t getSize() const override
    {
        if (_pSource)
        {
            return _pSource->size();
        }
        if (_pDrain)
        {
            return _pDrain->size();
        }

        return 0;
    }

private:
    ::etl::span<uint8_t const> const* _pSource;
    ::etl::span<uint8_t>* _pDrain;
};

} // namespace someip
