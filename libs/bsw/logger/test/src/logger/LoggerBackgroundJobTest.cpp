// Copyright 2024 Accenture.

#include "logger/LoggerBackgroundJob.h"

#include <util/stream/StringBufferOutputStream.h>

#include <gtest/gtest.h>

using namespace bsp;
using namespace logger;

namespace
{
struct LoggerBackgroundJobTest
: ::testing::Test
, IBackgroundJobHandler
, IEntryOutput<uint32_t, uint32_t>
{
    using EntryIndexType = uint32_t;
    using EntryRefType   = uint32_t;
    using TimestampType  = uint32_t;

    bool addJob(AbstractBackgroundJob& job) override
    {
        _job = &job;
        return true;
    }

    bool removeJob(AbstractBackgroundJob& /*job*/) override
    {
        _job = nullptr;
        return true;
    }

    bool outputEntry(IEntryOutput<uint32_t, uint32_t>& output, uint32_t& entryRef)
    {
        EXPECT_EQ(this, &output);
        _entryRef = ++entryRef;
        return _entryRef < 5U;
    }

    void outputEntry(
        uint32_t entryIndex,
        uint32_t timestamp,
        ::util::logger::ComponentInfo const& componentInfo,
        ::util::logger::LevelInfo const& levelInfo,
        char const* str,
        ::util::format::IPrintfArgumentReader& argReader) override
    {}

    AbstractBackgroundJob* _job = nullptr;
    uint32_t _entryRef          = 0;
};

TEST_F(LoggerBackgroundJobTest, testInitAndShutdown)
{
    ::util::stream::declare::StringBufferOutputStream<200> outputStream;
    LoggerBackgroundJob<LoggerBackgroundJobTest> cut(*this, *this, *this);
    ASSERT_EQ(nullptr, _job);
    cut.init();
    ASSERT_EQ(&cut, _job);
    ASSERT_EQ(0U, _entryRef);
    cut.execute();
    ASSERT_EQ(1U, _entryRef);
    cut.execute();
    ASSERT_EQ(2U, _entryRef);
    cut.shutdown();
    ASSERT_EQ(nullptr, _job);
    cut.flush();
    ASSERT_EQ(5U, _entryRef);
}

} // namespace
