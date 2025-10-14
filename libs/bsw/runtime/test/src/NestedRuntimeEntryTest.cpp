// Copyright 2024 Accenture.

#include "runtime/NestedRuntimeEntry.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;
using namespace ::runtime;

class TestStatistics
{
public:
    MOCK_METHOD3(addRun, void(uint32_t, uint32_t, uint32_t));
};

class NestedTestEntry
: public RuntimeStackEntry<NestedTestEntry, false>
, public TestStatistics
{};

class TestEntry : public NestedRuntimeEntry<TestEntry, TestStatistics, true, NestedTestEntry>
{};

TEST(NestedRuntimeEntryTest, testAll)
{
    StrictMock<TestEntry> entry1;
    StrictMock<TestEntry> entry2;
    StrictMock<NestedTestEntry> nestedEntry1;
    StrictMock<NestedTestEntry> nestedEntry2;

    entry1.push(50U, 0L);
    {
        entry2.push(100U, &entry1);
        EXPECT_CALL(entry2, addRun(100U, 200U, 0U));
        entry2.pop(300U);
        Mock::VerifyAndClearExpectations(&entry2);
    }
    {
        // add two nested entries and pop
        entry1.pushEntry(nestedEntry1, 400U);
        entry1.pushEntry(nestedEntry2, 450U);
        EXPECT_CALL(entry1, addRun(50U, 250U, 200U));
        entry1.pop(500U);
    }
    {
        // add the entry again and pop the nested entries
        entry1.push(750U, 0L);
        EXPECT_CALL(nestedEntry2, addRun(450U, 100U, 250U));
        entry1.popEntry(nestedEntry2, 800U);
        EXPECT_CALL(nestedEntry1, addRun(400U, 180U, 250U));
        entry1.popEntry(nestedEntry1, 830U);
    }
}

} // namespace
