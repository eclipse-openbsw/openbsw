// Copyright 2024 Accenture.

#include "util/crc/Crc32.h"

#include "fixtures/crc/CrcTestFixture.h"

#include <gmock/gmock.h>

namespace
{
using ::test::fixtures::CrcTestFixture;
using namespace ::util::crc;
using namespace ::util;
using namespace ::testing;

class Crc32Test : public CrcTestFixture
{};

TEST_F(Crc32Test, ccitt)
{
    Crc32::Ethernet crcRegister;

    crcRegister.init();
    crcRegister.update(_one_byte, sizeof(_one_byte));
    ASSERT_EQ(crcRegister.digest(), 0x83DCEFB7U);

    crcRegister.init();
    crcRegister.update(_one_zero_byte, sizeof(_one_zero_byte));
    ASSERT_EQ(crcRegister.digest(), 0xD202EF8DU);

    crcRegister.init();
    crcRegister.update(_multiple_bytes, sizeof(_multiple_bytes));
    ASSERT_EQ(crcRegister.digest(), 0xCBF43926U);

    crcRegister.init();
    crcRegister.update(_multiple_zero_bytes, sizeof(_multiple_zero_bytes));
    ASSERT_EQ(crcRegister.digest(), 0xE60914AEU);
}

TEST_F(Crc32Test, ARE2EP4)
{
    Crc32::ARE2EP4 crcRegister;

    crcRegister.init();
    crcRegister.update(_one_byte, sizeof(_one_byte));
    ASSERT_EQ(crcRegister.digest(), 0x2DE7AF5EU);

    crcRegister.init();
    crcRegister.update(_one_zero_byte, sizeof(_one_zero_byte));
    ASSERT_EQ(crcRegister.digest(), 0x6016DC99U);

    crcRegister.init();
    crcRegister.update(_multiple_bytes, sizeof(_multiple_bytes));
    ASSERT_EQ(crcRegister.digest(), 0x1697D06AU);

    crcRegister.init();
    crcRegister.update(_multiple_zero_bytes, sizeof(_multiple_zero_bytes));
    ASSERT_EQ(crcRegister.digest(), 0x4D43C7AAU);
}

} // anonymous namespace
