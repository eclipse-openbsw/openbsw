// Copyright 2024 Accenture.

#ifndef GUARD_A4F4C0F3_63D2_42F8_BD9E_1EA685931FA3
#define GUARD_A4F4C0F3_63D2_42F8_BD9E_1EA685931FA3

#include "io/IReader.h"

#include <gmock/gmock.h>

namespace io
{
class IReaderMock : public IReader
{
public:
    MOCK_CONST_METHOD0(maxSize, size_t());

    MOCK_CONST_METHOD0(peek, ::estd::slice<uint8_t>());

    MOCK_METHOD0(release, void());
};

} // namespace io

#endif // GUARD_A4F4C0F3_63D2_42F8_BD9E_1EA685931FA3
