// Copyright 2024 Accenture.

#include "runtime/RuntimeStackEntry.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;
using namespace ::runtime;

class TestCutOutEntry : public RuntimeStackEntry<TestCutOutEntry, true>
{
public:
    MOCK_METHOD3(addRun, void(uint32_t, uint32_t, uint32_t));
};

class TestNonCutOutEntry : public RuntimeStackEntry<TestNonCutOutEntry, false>
{
public:
    MOCK_METHOD3(addRun, void(uint32_t, uint32_t, uint32_t));
};

TEST(RuntimeStackEntryTest, testCutOutEntry)
{
    StrictMock<TestCutOutEntry> entry1;
    StrictMock<TestCutOutEntry> entry2;
    StrictMock<TestCutOutEntry> entry3;

    EXPECT_FALSE(entry1.isUsed());
    entry1.push(100U, 0L);
    EXPECT_TRUE(entry1.isUsed());
    EXPECT_FALSE(entry2.isUsed());
    entry2.push(200U, &entry1);
    EXPECT_TRUE(entry2.isUsed());
    {
        // push and remove entry
        EXPECT_FALSE(entry3.isUsed());
        entry3.push(400U, &entry2);
        EXPECT_TRUE(entry3.isUsed());
        EXPECT_CALL(entry3, addRun(400U, 200U, 0U));
        entry3.pop(600U);
        EXPECT_FALSE(entry3.isUsed());
        Mock::VerifyAndClearExpectations(&entry3);
    }
    {
        // push and remove entry again
        EXPECT_FALSE(entry3.isUsed());
        entry3.push(750U, &entry2);
        EXPECT_TRUE(entry3.isUsed());
        EXPECT_CALL(entry3, addRun(750U, 250U, 0U));
        entry3.pop(1000U);
        EXPECT_FALSE(entry3.isUsed());
        Mock::VerifyAndClearExpectations(&entry3);
    }
    {
        // expect cut out time when removing entry 2
        EXPECT_CALL(entry2, addRun(200U, 650U, 450U));
        entry2.pop(1300U);
        EXPECT_FALSE(entry2.isUsed());
        Mock::VerifyAndClearExpectations(&entry2);
    }
    {
        // expect cut out time when removing entry 1
        EXPECT_CALL(entry1, addRun(100U, 250U, 1100U));
        entry1.pop(1450U);
        EXPECT_FALSE(entry1.isUsed());
        Mock::VerifyAndClearExpectations(&entry1);
    }
}

TEST(RuntimeStackEntryTest, testNonCutOutEntry)
{
    StrictMock<TestNonCutOutEntry> entry1;
    StrictMock<TestNonCutOutEntry> entry2;
    StrictMock<TestNonCutOutEntry> entry3;

    EXPECT_FALSE(entry1.isUsed());
    entry1.push(100U, 0L);
    EXPECT_TRUE(entry1.isUsed());
    {
        // push again
        entry1.push(500U, 0L);
    }
    EXPECT_FALSE(entry2.isUsed());
    entry2.push(200U, &entry1);
    EXPECT_TRUE(entry2.isUsed());
    {
        // push and remove entry
        EXPECT_FALSE(entry3.isUsed());
        entry3.push(400U, &entry2);
        EXPECT_TRUE(entry3.isUsed());
        EXPECT_CALL(entry3, addRun(400U, 200U, 0U));
        entry3.pop(600U);
        EXPECT_FALSE(entry3.isUsed());
        Mock::VerifyAndClearExpectations(&entry3);
    }
    {
        // push and remove entry again
        EXPECT_FALSE(entry3.isUsed());
        entry3.push(750U, &entry2);
        EXPECT_TRUE(entry3.isUsed());
        EXPECT_CALL(entry3, addRun(750U, 250U, 0U));
        entry3.pop(1000U);
        EXPECT_FALSE(entry3.isUsed());
        Mock::VerifyAndClearExpectations(&entry3);
    }
    {
        // expect cut out time when removing entry 2
        EXPECT_CALL(entry2, addRun(200U, 1100U, 0U));
        entry2.pop(1300U);
        EXPECT_FALSE(entry2.isUsed());
        Mock::VerifyAndClearExpectations(&entry2);
    }
    {
        // expect cut out time when removing entry 1
        EXPECT_CALL(entry1, addRun(100U, 1350U, 0U));
        entry1.pop(1450U);
        EXPECT_FALSE(entry1.isUsed());
        Mock::VerifyAndClearExpectations(&entry1);
    }
}

} // namespace
