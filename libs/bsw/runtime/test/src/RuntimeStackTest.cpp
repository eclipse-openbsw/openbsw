// Copyright 2024 Accenture.

#include "runtime/RuntimeStack.h"

#include "runtime/RuntimeStackEntry.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;
using namespace ::runtime;

class TestEntry : public RuntimeStackEntry<TestEntry, true>
{
public:
    MOCK_METHOD(void, addRun, (uint32_t, uint32_t, uint32_t));
};

TEST(RuntimeStackTest, testPushPop)
{
    StrictMock<TestEntry> entry1;
    StrictMock<TestEntry> entry2;
    StrictMock<TestEntry> entry3;

    RuntimeStack<TestEntry> cut;

    EXPECT_EQ(0L, cut.getTopEntry());
    cut.pushEntry(entry1, 100U);
    EXPECT_EQ(&entry1, cut.getTopEntry());
    cut.pushEntry(entry2, 200U);
    EXPECT_EQ(&entry2, cut.getTopEntry());
    {
        // removing first entry should do nothing
        cut.popEntry(entry1, 1000U);
    }
    EXPECT_EQ(&entry2, cut.getTopEntry());
    {
        // adding second entry again should do nothing
        cut.pushEntry(entry2, 300U);
    }
    EXPECT_EQ(&entry2, cut.getTopEntry());
    {
        cut.pushEntry(entry3, 500U);
        EXPECT_CALL(entry3, addRun(500U, 400U, 0U));
        cut.popEntry(entry3, 900U);
        Mock::VerifyAndClearExpectations(&entry3);
    }
    EXPECT_EQ(&entry2, cut.getTopEntry());
    {
        cut.pushEntry(entry3, 1000U);
        EXPECT_CALL(entry3, addRun(1000U, 100U, 0U));
        cut.popEntry(entry3, 1100U);
        Mock::VerifyAndClearExpectations(&entry3);
    }
    EXPECT_EQ(&entry2, cut.getTopEntry());
    {
        EXPECT_CALL(entry2, addRun(200U, 600U, 500U));
        cut.popEntry(entry2, 1300U);
        Mock::VerifyAndClearExpectations(&entry2);
    }
    EXPECT_EQ(&entry1, cut.getTopEntry());
    {
        // add suspended time
        cut.addSuspendedTime(30U);
        EXPECT_CALL(entry1, addRun(100U, 270U, 1130U));
        cut.popEntry(entry1, 1500U);
        Mock::VerifyAndClearExpectations(&entry1);
    }
    EXPECT_EQ(0L, cut.getTopEntry());
}

TEST(RuntimeStackTest, testPopTopEntry)
{
    StrictMock<TestEntry> entry;

    RuntimeStack<TestEntry> cut;
    cut.pushEntry(entry, 100U);
    {
        // removing top entry should work immediately
        EXPECT_CALL(entry, addRun(100U, 200U, 0U));
        cut.popEntry(300U);
        Mock::VerifyAndClearExpectations(&entry);
    }
    {
        // removing entry from empty stack should cause nothing
        cut.popEntry(300U);
        Mock::VerifyAndClearExpectations(&entry);
    }
}

} // namespace
