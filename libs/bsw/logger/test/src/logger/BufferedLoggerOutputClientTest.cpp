// Copyright 2024 Accenture.

#include "logger/BufferedLoggerOutputClient.h"

#include <gtest/gtest.h>

namespace
{
using namespace logger;

struct BufferedLoggerOutputClientTest
: ::testing::Test
, IEntryOutput<uint32_t, uint32_t>
, ILoggerListener
{
    using EntryIndexType = uint32_t;
    using EntryRefType   = uint32_t;
    using TimestampType  = uint32_t;

    void addListener(ILoggerListener& listener) { _listener = &listener; }

    void removeListener(ILoggerListener& listener)
    {
        ASSERT_EQ(_listener, &listener);
        _listener = nullptr;
    }

    bool outputEntry(IEntryOutput<EntryIndexType, TimestampType>& output, EntryRefType& entryRef)
    {
        _output     = &output;
        _entryRefIn = entryRef;
        entryRef    = _entryRefOut;
        return _result;
    }

    void outputEntry(
        uint32_t entryIndex,
        uint32_t timestamp,
        ::util::logger::ComponentInfo const& componentInfo,
        ::util::logger::LevelInfo const& levelInfo,
        char const* str,
        ::util::format::IPrintfArgumentReader& argReader) override
    {}

    void logAvailable() override {}

    IEntryOutput<EntryIndexType, TimestampType>* getOutput() const { return _output; }

    EntryRefType getEntryRefIn() const { return _entryRefIn; }

    ILoggerListener* getListener() const { return _listener; }

    void setOutputEntryMembers(EntryRefType entryRef, bool result)
    {
        _entryRefOut = entryRef;
        _result      = result;
    }

    IEntryOutput<EntryIndexType, TimestampType>* _output = nullptr;
    ILoggerListener* _listener                           = nullptr;
    uint32_t _entryRefIn                                 = 0;
    uint32_t _entryRefOut                                = 0;
    bool _result                                         = false;
};

TEST_F(BufferedLoggerOutputClientTest, testAddRemoveListener)
{
    BufferedLoggerOutputClient<BufferedLoggerOutputClientTest> cut(*this);
    cut.addListener(*this);
    ASSERT_EQ(this, getListener());
    cut.removeListener(*this);
    ASSERT_EQ(nullptr, getListener());
}

TEST_F(BufferedLoggerOutputClientTest, testOutputEntry)
{
    BufferedLoggerOutputClient<BufferedLoggerOutputClientTest> cut(*this);
    setOutputEntryMembers(1, false);
    ASSERT_FALSE(cut.outputEntry(*this));
    ASSERT_EQ(this, getOutput());
    ASSERT_EQ(0U, getEntryRefIn());
    setOutputEntryMembers(2, true);
    ASSERT_TRUE(cut.outputEntry(*this));
    ASSERT_EQ(this, getOutput());
    ASSERT_EQ(1U, getEntryRefIn());
}

} // namespace
