// Copyright 2024 Accenture.

#ifndef GUARD_E704BBC3_516C_40E1_BC6A_EAD7C443EFEB
#define GUARD_E704BBC3_516C_40E1_BC6A_EAD7C443EFEB

#include "bsp/flash/IFlash.h"

#include <gmock/gmock.h>

namespace bsp
{
namespace flash
{
struct FlashMock : IFlash
{
    MOCK_CONST_METHOD0(memory, ::estd::slice<uint8_t const>());
    MOCK_METHOD2(write, uint32_t(uint32_t, ::estd::slice<uint8_t const>));
    MOCK_METHOD0(flush, bool());
    MOCK_METHOD1(erase, bool(FlashBlock const&));
    MOCK_CONST_METHOD2(block, FlashBlock(uint32_t, uint32_t));
};

} // namespace flash
} // namespace bsp

#endif /* GUARD_E704BBC3_516C_40E1_BC6A_EAD7C443EFEB */
