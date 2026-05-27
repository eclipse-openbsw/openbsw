/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/crc/Crc8.h"

#include "fixtures/crc/CrcTestFixture.h"

#include <gmock/gmock.h>

namespace
{
using ::test::fixtures::CrcTestFixture;
using namespace ::util::crc;
using namespace ::util;
using namespace ::testing;

class Crc8Test : public CrcTestFixture
{};

TEST_F(Crc8Test, ccitt)
{
    Crc8::Ccitt crcRegister;

    crcRegister.init();
    crcRegister.update(_one_byte, sizeof(_one_byte));
    ASSERT_EQ(crcRegister.digest(), 0x97U);

    crcRegister.init();
    crcRegister.update(_one_zero_byte, sizeof(_one_zero_byte));
    ASSERT_EQ(crcRegister.digest(), 0x00);

    crcRegister.init();
    crcRegister.update(_multiple_bytes, sizeof(_multiple_bytes));
    ASSERT_EQ(crcRegister.digest(), 0xF4U);

    crcRegister.init();
    crcRegister.update(_multiple_zero_bytes, sizeof(_multiple_zero_bytes));
    ASSERT_EQ(crcRegister.digest(), 0x00U);
}

TEST_F(Crc8Test, saej1850)
{
    Crc8::Saej1850 crcRegister;

    crcRegister.init();
    crcRegister.update(_one_byte, sizeof(_one_byte));
    ASSERT_EQ(crcRegister.digest(), 0x57U);

    crcRegister.init();
    crcRegister.update(_one_zero_byte, sizeof(_one_zero_byte));
    ASSERT_EQ(crcRegister.digest(), 0x00);

    crcRegister.init();
    crcRegister.update(_multiple_bytes, sizeof(_multiple_bytes));
    ASSERT_EQ(crcRegister.digest(), 0x37U);

    crcRegister.init();
    crcRegister.update(_multiple_zero_bytes, sizeof(_multiple_zero_bytes));
    ASSERT_EQ(crcRegister.digest(), 0x00U);
}

TEST_F(Crc8Test, maxim)
{
    Crc8::Maxim crcRegister;

    crcRegister.init();
    crcRegister.update(_one_byte, sizeof(_one_byte));
    ASSERT_EQ(crcRegister.digest(), 0xE0U);

    crcRegister.init();
    crcRegister.update(_one_zero_byte, sizeof(_one_zero_byte));
    ASSERT_EQ(crcRegister.digest(), 0x00U);

    crcRegister.init();
    crcRegister.update(_multiple_bytes, sizeof(_multiple_bytes));
    ASSERT_EQ(crcRegister.digest(), 0xA1U);

    crcRegister.init();
    crcRegister.update(_multiple_zero_bytes, sizeof(_multiple_zero_bytes));
    ASSERT_EQ(crcRegister.digest(), 0x00U);
}

TEST_F(Crc8Test, Crc8H2F)
{
    Crc8::Crc8H2F crcRegister;

    crcRegister.init();
    crcRegister.update(_one_byte, sizeof(_one_byte));
    ASSERT_EQ(crcRegister.digest(), 0x4FU);

    crcRegister.init();
    crcRegister.update(_one_zero_byte, sizeof(_one_zero_byte));
    ASSERT_EQ(crcRegister.digest(), 0xBDU);

    crcRegister.init();
    crcRegister.update(_multiple_bytes, sizeof(_multiple_bytes));
    ASSERT_EQ(crcRegister.digest(), 0xDFU);

    crcRegister.init();
    crcRegister.update(_multiple_zero_bytes, sizeof(_multiple_zero_bytes));
    ASSERT_EQ(crcRegister.digest(), 0xE1U);
}

TEST_F(Crc8Test, Rohc)
{
    Crc8::Rohc crcRegister;

    crcRegister.init();
    crcRegister.update(_one_byte, sizeof(_one_byte));
    ASSERT_EQ(crcRegister.digest(), 0x7AU);

    crcRegister.init();
    crcRegister.update(_one_zero_byte, sizeof(_one_zero_byte));
    ASSERT_EQ(crcRegister.digest(), 0xCFU);

    crcRegister.init();
    crcRegister.update(_multiple_bytes, sizeof(_multiple_bytes));
    ASSERT_EQ(crcRegister.digest(), 0xD0U);

    crcRegister.init();
    crcRegister.update(_multiple_zero_bytes, sizeof(_multiple_zero_bytes));
    ASSERT_EQ(crcRegister.digest(), 0xF0U);
}

} // anonymous namespace
