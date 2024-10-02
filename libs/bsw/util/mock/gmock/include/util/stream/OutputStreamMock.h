// Copyright 2024 Accenture.

#ifndef GUARD_011E43CB_A312_42D1_BC33_EA7712BFCB2F
#define GUARD_011E43CB_A312_42D1_BC33_EA7712BFCB2F

#include <util/stream/IOutputStream.h>

#include <gmock/gmock.h>

namespace util
{
namespace stream
{
class OutputStreamMock : public IOutputStream
{
public:
    MOCK_CONST_METHOD0(isEof, bool());
    MOCK_METHOD1(write, void(uint8_t data));
    MOCK_METHOD1(write, void(::estd::slice<uint8_t const> const& buffer));
};

} // namespace stream
} // namespace util

#endif /* GUARD_011E43CB_A312_42D1_BC33_EA7712BFCB2F */
