// Copyright 2024 Accenture.

#include "uds/services/routinecontrol/StopRoutine.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;

class MyStopRoutine : public StopRoutine
{
public:
    virtual DiagReturnCode::Type getDefaultDiagReturnCode() const
    {
        return StopRoutine::getDefaultDiagReturnCode();
    }
};

TEST(
    StopRoutineTest,
    a_StopRoutine_object_should_set_DefaultDiagReturnCode_to_ISO_REQUEST_OUT_OF_RANGE)
{
    MyStopRoutine myStopRoutine;

    EXPECT_EQ(DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE, myStopRoutine.getDefaultDiagReturnCode());
}

} // anonymous namespace
