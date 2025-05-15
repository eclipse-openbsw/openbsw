// Copyright 2024 Accenture.

#pragma once

#include "io/IWriter.h"

#include <gmock/gmock.h>

namespace io
{
class IWriterMock : public IWriter
{
public:
    MOCK_METHOD(size_t, maxSize, (), (const));

    MOCK_METHOD(::etl::span<uint8_t>, allocate, (size_t));

    MOCK_METHOD(void, commit, ());

    MOCK_METHOD(void, flush, ());
};

} // namespace io
