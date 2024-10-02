// Copyright 2024 Accenture.

#include "util/crc/Crc.h"

#include <estd/slice.h>

#include <gmock/gmock.h>

using namespace ::testing;

TEST(CrcRegister, update_using_slice)
{
    // test setup
    uint8_t data[9] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    using Crc8      = ::util::crc::CrcRegister<uint8_t, 0x07U>;

    // run test scenario
    Crc8 crcRegister;
    crcRegister.update(::estd::make_slice(data));

    // assert expectations
    ASSERT_EQ(crcRegister.digest(), 0xF4U);
}
