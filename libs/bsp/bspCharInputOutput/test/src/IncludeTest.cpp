/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

// gtest conflicts with some GCC versions
#define sprintf   GCC_UT_TEMP1
#define vsprintf  GCC_UT_TEMP2
#define snprintf  GCC_UT_TEMP3
#define vsnprintf GCC_UT_TEMP4
#include "charInputOutput/bspIoStd.h"
#define GCC_UT_TEMP1 sprintf
#define GCC_UT_TEMP2 vsprintf
#define GCC_UT_TEMP3 snprintf
#define GCC_UT_TEMP4 vsnprintf

#include "charInputOutput/charIo.h"
#include "charInputOutput/charIoSerial.h"

#include <gtest/gtest.h>

namespace
{
TEST(bspCharInputOutput, IncludeTest)
{
    SUCCEED();
    // This test currently only checks if the includes are working.
}

} // anonymous namespace
