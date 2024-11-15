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
    MOCK_CONST_METHOD0(maxSize, size_t());

    MOCK_CONST_METHOD0(peek, ::etl::span<uint8_t>());

    MOCK_METHOD0(release, void());
};

} // namespace io
