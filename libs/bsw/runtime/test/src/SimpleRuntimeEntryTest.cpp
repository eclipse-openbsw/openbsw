// Copyright 2024 Accenture.

#include "runtime/SimpleRuntimeEntry.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;
using namespace ::runtime;

class TestStatistics
{
public:
    MOCK_METHOD(void, addRun, (uint32_t, uint32_t, uint32_t));
};

class TestEntry : public SimpleRuntimeEntry<TestEntry, TestStatistics, true>
{};

TEST(SimpleRuntimeEntryTest, testAll)
{
    StrictMock<TestEntry> entry1;
    StrictMock<TestEntry> entry2;

    entry1.push(50U, 0L);
    {
        entry2.push(100U, &entry1);
        EXPECT_CALL(entry2, addRun(100U, 200U, 0U));
        entry2.pop(300U);
        Mock::VerifyAndClearExpectations(&entry2);
    }
    {
        entry2.push(380U, &entry1);
        EXPECT_CALL(entry2, addRun(380U, 20U, 0U));
        entry2.pop(400U);
        Mock::VerifyAndClearExpectations(&entry2);
    }
    {
        EXPECT_CALL(entry1, addRun(50U, 220U, 220U));
        entry1.pop(490U);
        Mock::VerifyAndClearExpectations(&entry1);
    }
}

} // namespace
