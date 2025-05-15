// Copyright 2024 Accenture.

#pragma once

#include "io/IReader.h"

#include <etl/span.h>

#include <gmock/gmock.h>

namespace io
{
class IReaderMock : public IReader
{
public:
    MOCK_METHOD(size_t, maxSize, (), (const));

    MOCK_METHOD(::etl::span<uint8_t>, peek, (), (const));

    MOCK_METHOD(void, release, ());
};

} // namespace io
