/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/stream/NullOutputStream.h"

#include <gtest/gtest.h>

using namespace ::util::stream;

TEST(NullOutputStream, testNothingIsDone)
{
    NullOutputStream cut;
    ASSERT_TRUE(cut.isEof());
    cut.write('a');
    cut.write_string_view(::etl::string_view("abc\ndef"));
}
