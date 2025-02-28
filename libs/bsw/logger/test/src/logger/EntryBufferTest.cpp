// Copyright 2024 Accenture.

#include "logger/EntryBuffer.h"

#include <gtest/gtest.h>

#include <vector>

using namespace ::logger;

struct EntryBufferTest : ::testing::Test
{
    template<class B>
    void checkNextEntry(
        B& buffer, typename B::EntryRef& entryRef, int32_t expectedIdx = -1, int32_t entrySize = -1)
    {
        ::estd::slice<uint8_t> const* pEntry = nullptr;
        if (expectedIdx >= 0 && expectedIdx < int32_t(fEntries.size()))
        {
            pEntry = &(*(fEntries.begin() + expectedIdx));
        }
        uint32_t bufferSize = entrySize >= 0 ? entrySize : (pEntry ? pEntry->size() : 0);
        std::vector<uint8_t> readVector(bufferSize + 2);
        uint8_t* pReadBuffer = &(readVector[0]);
        memset(pReadBuffer, 0xaf, bufferSize + 2);
        ASSERT_EQ(
            bufferSize,
            buffer.getNextEntry(
                ::estd::slice<uint8_t>::from_pointer(pReadBuffer + 1, bufferSize), entryRef));
        EXPECT_EQ(0xafU, pReadBuffer[0]);
        EXPECT_EQ(0xafU, pReadBuffer[bufferSize + 1]);
        if (pEntry)
        {
            ASSERT_EQ(0, memcmp(pReadBuffer + 1, pEntry->data(), bufferSize));
        }
    }

    template<class B>
    void pushEntry(B& buffer, uint8_t size)
    {
        uint8_t* pEntry = new uint8_t[size];
        uint8_t value   = size * sizeof(fEntries);
        for (uint8_t i = 0; i < size; ++i)
        {
            pEntry[i] = value++;
        }
        fEntries.push_back(::estd::slice<uint8_t>::from_pointer(pEntry, size));
        buffer.addEntry(fEntries.back());
    }

    ~EntryBufferTest()
    {
        for (auto const& fEntrie : fEntries)
        {
            delete[] fEntrie.data();
        }
    }

    std::vector<::estd::slice<uint8_t>> fEntries;
};

TEST_F(EntryBufferTest, testEntryRefConstructorAndAssignment)
{
    {
        EntryBuffer<>::EntryRef cut;
        ASSERT_EQ(0U, cut.getIndex());
        ASSERT_EQ(nullptr, cut.getReadPointer());
    }
    {
        uint8_t array[128];
        EntryBuffer<> entryBuffer(array);
        pushEntry(entryBuffer, 45);
        EntryBuffer<>::EntryRef src;
        uint8_t readArray[10];
        entryBuffer.getNextEntry(readArray, src);
        {
            EntryBuffer<>::EntryRef cut(src);
            ASSERT_EQ(cut.getIndex(), src.getIndex());
            ASSERT_EQ(cut.getReadPointer(), src.getReadPointer());
        }
        {
            EntryBuffer<>::EntryRef cut;
            cut = src;
            ASSERT_EQ(cut.getIndex(), src.getIndex());
            ASSERT_EQ(cut.getReadPointer(), src.getReadPointer());
        }
        {
            EntryBuffer<>::EntryRef cut = src;
            cut                         = *&cut;
            ASSERT_EQ(cut.getIndex(), src.getIndex());
            ASSERT_EQ(cut.getReadPointer(), src.getReadPointer());
        }
    }
}

TEST_F(EntryBufferTest, testInitiallyNoNextEntry)
{
    uint8_t buffer[300];
    EntryBuffer<> cut(buffer);
    EntryBuffer<>::EntryRef entryRef;
    checkNextEntry(cut, entryRef);
}

TEST_F(EntryBufferTest, testFirstEntryIsReportedToTwoReferences)
{
    uint8_t buffer[300];
    EntryBuffer<> cut(buffer);
    pushEntry(cut, 45);
    EntryBuffer<>::EntryRef entryRef1;
    checkNextEntry(cut, entryRef1, 0);
    checkNextEntry(cut, entryRef1);
    EntryBuffer<>::EntryRef entryRef2;
    checkNextEntry(cut, entryRef2, 0);
    checkNextEntry(cut, entryRef2);
}

TEST_F(EntryBufferTest, testEntriesAreCutOnMaximumEntrySize)
{
    uint8_t buffer[300];
    EntryBuffer<> cut(buffer);
    pushEntry(cut, 65);
    pushEntry(cut, 64);
    EntryBuffer<>::EntryRef entryRef;
    checkNextEntry(cut, entryRef, 0, 64);
    checkNextEntry(cut, entryRef, 1);
    checkNextEntry(cut, entryRef);
}

TEST_F(EntryBufferTest, testWrapAroundBeforeNewEntry)
{
    uint8_t buffer[128];
    EntryBuffer<> cut(buffer);
    pushEntry(cut, 63);
    pushEntry(cut, 63);
    pushEntry(cut, 63);
    EntryBuffer<>::EntryRef entryRef;
    checkNextEntry(cut, entryRef, 1);
    checkNextEntry(cut, entryRef, 2);
    checkNextEntry(cut, entryRef);
}

TEST_F(EntryBufferTest, testWrapAroundBeforeLengthByteOfSecondEntry)
{
    uint8_t buffer[128];
    EntryBuffer<> cut(buffer);
    pushEntry(cut, 63);
    pushEntry(cut, 62);
    pushEntry(cut, 63);
    EntryBuffer<>::EntryRef entryRef;
    checkNextEntry(cut, entryRef, 1);
    checkNextEntry(cut, entryRef, 2);
    checkNextEntry(cut, entryRef);
}

TEST_F(EntryBufferTest, testWrapAroundWithinEntry)
{
    uint8_t buffer[128];
    EntryBuffer<> cut(buffer);
    pushEntry(cut, 60);
    pushEntry(cut, 60);
    pushEntry(cut, 60);
    EntryBuffer<>::EntryRef entryRef;
    checkNextEntry(cut, entryRef, 1);
    checkNextEntry(cut, entryRef, 2);
    checkNextEntry(cut, entryRef);
}

TEST_F(EntryBufferTest, testEdgeCasesForEntry)
{
    uint8_t buffer[128];
    EntryBuffer<> cut(buffer);
    pushEntry(cut, 60);
    EntryBuffer<>::EntryRef entryRef;
    uint8_t readBuffer[10];
    ASSERT_EQ(10, cut.getNextEntry(readBuffer, entryRef));
    pushEntry(cut, 80);
    pushEntry(cut, 80);
    checkNextEntry(cut, entryRef, 3);
}
