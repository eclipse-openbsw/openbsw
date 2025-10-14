// Copyright 2024 Accenture.

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
