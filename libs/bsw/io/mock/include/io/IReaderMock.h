// Copyright 2024 Accenture.

#pragma once

#include "io/IReader.h"

#include <gmock/gmock.h>

namespace io
{
class IReaderMock : public IReader
{
public:
    MOCK_METHOD(size_t, maxSize, (), (const, override));

    MOCK_METHOD(::estd::slice<uint8_t>, peek, (), (const, override));

    MOCK_METHOD(void, release, (), (override));
};

} // namespace io
