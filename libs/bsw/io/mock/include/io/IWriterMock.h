// Copyright 2024 Accenture.

#ifndef GUARD_03759470_7328_4ABD_BF30_A922B7D97024
#define GUARD_03759470_7328_4ABD_BF30_A922B7D97024

#include "io/IWriter.h"

#include <gmock/gmock.h>

namespace io
{
class IWriterMock : public IWriter
{
public:
    MOCK_CONST_METHOD0(maxSize, size_t());

    MOCK_METHOD1(allocate, ::estd::slice<uint8_t>(size_t));

    MOCK_METHOD0(commit, void());

    MOCK_METHOD0(flush, void());
};

} // namespace io

#endif // GUARD_03759470_7328_4ABD_BF30_A922B7D97024
