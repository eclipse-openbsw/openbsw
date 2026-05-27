/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/services/routinecontrol/RequestRoutineResults.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;

class MyRequestRoutineResults : public RequestRoutineResults
{
public:
    virtual DiagReturnCode::Type getDefaultDiagReturnCode() const
    {
        return RequestRoutineResults::getDefaultDiagReturnCode();
    }
};

TEST(
    RequestRoutineResultsTest,
    a_RequestRoutineResults_object_should_set_DefaultDiagReturnCode_to_ISO_REQUEST_OUT_OF_RANGE)
{
    MyRequestRoutineResults myRequestRoutineResults;

    EXPECT_EQ(
        DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE,
        myRequestRoutineResults.getDefaultDiagReturnCode());
}

} // anonymous namespace
