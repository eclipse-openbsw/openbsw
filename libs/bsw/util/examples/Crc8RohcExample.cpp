// Copyright 2024 Accenture.

#include "fixtures/crc/CrcTestFixture.h"
#include "util/crc/Crc.h"

#include <gtest/gtest.h>

namespace
{
void oneshot_crc_8_rohc()
{
    // EXAMPLE_START crc8rohc
    CrcRegister<uint8_t, 0x07, 0xff, true, true, 0> oneshotCrcRohc;

    oneshotCrcRohc.init();
    oneshotCrcRohc.update(_one_byte, sizeof(_one_byte));
    ASSERT_EQ(oneshotCrcRohc.digest(), 0x7AU);

    oneshotCrcRohc.init();
    oneshotCrcRohc.update(_one_zero_byte, sizeof(_one_zero_byte));
    ASSERT_EQ(oneshotCrcRohc.digest(), 0xCFU);

    oneshotCrcRohc.init();
    oneshotCrcRohc.update(_multiple_bytes, sizeof(_multiple_bytes));
    ASSERT_EQ(oneshotCrcRohc.digest(), 0xD0U);

    oneshotCrcRohc.init();
    oneshotCrcRohc.update(_multiple_zero_bytes, sizeof(_multiple_zero_bytes));
    ASSERT_EQ(oneshotCrcRohc.digest(), 0xF0U);
    // EXAMPLE_END crc8rohc
}

} // namespace

TEST(Crc, run_examples) { oneshot_crc_8_rohc(); }
