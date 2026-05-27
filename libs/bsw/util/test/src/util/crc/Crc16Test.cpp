/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/crc/Crc16.h"

#include "fixtures/crc/CrcTestFixture.h"

#include <gmock/gmock.h>

namespace
{
using ::test::fixtures::CrcTestFixture;
using namespace ::util::crc;
using namespace ::util;
using namespace ::testing;

class Crc16Test : public CrcTestFixture
{};

TEST_F(Crc16Test, ccitt)
{
    Crc16::Ccitt crcRegister;

    crcRegister.init();
    crcRegister.update(_one_byte, sizeof(_one_byte));
    ASSERT_EQ(crcRegister.digest(), 0xC782U);

    crcRegister.init();
    crcRegister.update(_one_zero_byte, sizeof(_one_zero_byte));
    ASSERT_EQ(crcRegister.digest(), 0xE1F0U);

    crcRegister.init();
    crcRegister.update(_multiple_bytes, sizeof(_multiple_bytes));
    ASSERT_EQ(crcRegister.digest(), 0x29B1U);

    crcRegister.init();
    crcRegister.update(_multiple_zero_bytes, sizeof(_multiple_zero_bytes));
    ASSERT_EQ(crcRegister.digest(), 0x1872U);
}

} // anonymous namespace
