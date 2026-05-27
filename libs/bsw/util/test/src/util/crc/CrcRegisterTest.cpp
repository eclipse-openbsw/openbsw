/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/crc/Crc.h"

#include <etl/span.h>

#include <gmock/gmock.h>

using namespace ::testing;

TEST(CrcRegister, update_using_slice)
{
    // test setup
    uint8_t data[9] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    using Crc8      = ::util::crc::CrcRegister<uint8_t, 0x07U>;

    // run test scenario
    Crc8 crcRegister;
    crcRegister.update(::etl::span<uint8_t>(data));

    // assert expectations
    ASSERT_EQ(crcRegister.digest(), 0xF4U);
}
