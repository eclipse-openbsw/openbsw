/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/services/routinecontrol/StartRoutine.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;

class MyStartRoutine : public StartRoutine
{
public:
    virtual DiagReturnCode::Type getDefaultDiagReturnCode() const
    {
        return StartRoutine::getDefaultDiagReturnCode();
    }
};

TEST(
    StartRoutineTest,
    a_StartRoutine_object_should_set_DefaultDiagReturnCode_to_ISO_REQUEST_OUT_OF_RANGE)
{
    MyStartRoutine myStartRoutine;

    EXPECT_EQ(DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE, myStartRoutine.getDefaultDiagReturnCode());
}

} // anonymous namespace
