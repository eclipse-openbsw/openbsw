// Copyright 2024 Accenture.

#pragma once

#include "io/IWriter.h"

#include <gmock/gmock.h>

namespace io
{
class IWriterMock : public IWriter
{
public:
    MOCK_METHOD(size_t, maxSize, (), (const, override));

    MOCK_METHOD(::estd::slice<uint8_t>, allocate, (size_t), (override));

    MOCK_METHOD(void, commit, (), (override));

    MOCK_METHOD(void, flush, (), (override));
};

} // namespace io
