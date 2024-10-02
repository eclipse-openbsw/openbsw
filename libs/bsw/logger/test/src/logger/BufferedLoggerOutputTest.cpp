// Copyright 2024 Accenture.

#include "logger/BufferedLoggerOutput.h"

#include "logger/ComponentMapping.h"
#include "logger/ILoggerTime.h"

#include <util/format/StringWriter.h>
#include <util/stream/StringBufferOutputStream.h>

#include <gtest/gtest.h>

namespace util
{
namespace logger
{
static uint8_t CONF1 = COMPONENT_NONE;
static uint8_t CONF2 = COMPONENT_NONE;
static uint8_t CONF3 = COMPONENT_NONE;
} // namespace logger
} // namespace util

namespace
{
using namespace logger;
using namespace ::util;

uint32_t _totalLockCount;

struct TestLock
{
    TestLock() { ++_totalLockCount; }
};

struct BufferedLoggerOutputTest
: ::testing::Test
, ILoggerListener
, IEntryOutput<uint32_t, uint32_t>
, ILoggerTime<uint32_t>
{
    BufferedLoggerOutputTest() { _totalLockCount = 0; }

    void logAvailable() override { ++_availableLogCount; }

    void outputEntry(
        uint32_t entryIndex,
        uint32_t timestamp,
        ::util::logger::ComponentInfo const& componentInfo,
        ::util::logger::LevelInfo const& levelInfo,
        char const* str,
        format::IPrintfArgumentReader& argReader) override
    {
        stream::declare::StringBufferOutputStream<300> outputStream;
        format::StringWriter writer(outputStream);
        writer.printf(
            "%d %d %d %d ", entryIndex, timestamp, componentInfo.getIndex(), levelInfo.getLevel());
        writer.vprintf(str, argReader);
        _entryStr = outputStream.getString();
    }

    uint32_t getTimestamp() const override { return _timestamp; }

    void
    formatTimestamp(stream::IOutputStream& outputStream, uint32_t const& timestamp) const override
    {
        ASSERT_EQ(_timestamp, timestamp);
        format::StringWriter writer(outputStream);
        writer.printf("%d", timestamp);
    }

    template<class T>
    void callLogOutput(
        T& buffer,
        ::util::logger::ComponentInfo const& componentInfo,
        ::util::logger::LevelInfo const& levelInfo,
        char const* str,
        ...)
    {
        va_list ap;
        va_start(ap, str);
        buffer.logOutput(componentInfo, levelInfo, str, ap);
        va_end(ap);
    }

    void setTimestamp(uint32_t timestamp) { _timestamp = timestamp; }

    bool checkAndResetEntry(char const* expected)
    {
        bool result = _entryStr == expected;
        _entryStr.clear();
        return result;
    }

    uint32_t _timestamp         = 0;
    uint32_t _availableLogCount = 0;
    std::string _entryStr;
};

START_LOGGER_COMPONENT_MAPPING_INFO_TABLE(componentInfoTable)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_DEBUG, CONF1)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_DEBUG, CONF2)
LOGGER_COMPONENT_MAPPING_INFO(LEVEL_DEBUG, CONF3)
END_LOGGER_COMPONENT_MAPPING_INFO_TABLE();

DEFINE_LOGGER_COMPONENT_MAPPING(
    TestMappingType,
    testMapping,
    componentInfoTable,
    ::util::logger::LevelInfo::getDefaultTable(),
    CONF1);

TEST_F(BufferedLoggerOutputTest, testAll)
{
    declare::BufferedLoggerOutput<4096, TestLock> cut(testMapping, *this);
    cut.addListener(*this);
    ASSERT_EQ(0U, _totalLockCount);
    setTimestamp(2348);
    callLogOutput(
        cut,
        testMapping.getComponentInfo(1),
        testMapping.getLevelInfo(::util::logger::LEVEL_DEBUG),
        "format string %d %s",
        17,
        "218439");
    ASSERT_EQ(1U, _totalLockCount);
    BufferedLoggerOutput<TestLock>::EntryRefType entryRef;
    cut.outputEntry(*this, entryRef);
    ASSERT_TRUE(checkAndResetEntry("1 2348 1 0 format string 17 218439"));
    ASSERT_EQ(2U, _totalLockCount);
    cut.outputEntry(*this, entryRef);
    ASSERT_TRUE(checkAndResetEntry(""));
    ASSERT_EQ(3U, _totalLockCount);
    cut.removeListener(*this);
    ASSERT_EQ(3U, _totalLockCount);
}

TEST_F(BufferedLoggerOutputTest, testConstructorWithPredicate)
{
    declare::BufferedLoggerOutput<20, TestLock, 16> cut(
        testMapping, *this, SectionPredicate(nullptr, nullptr));
    setTimestamp(2348);
    callLogOutput(
        cut,
        testMapping.getComponentInfo(1),
        testMapping.getLevelInfo(::util::logger::LEVEL_DEBUG),
        "very long format string that cannot fit into a 20 bytes buffer");
    BufferedLoggerOutput<TestLock, 16>::EntryRefType entryRef;
    cut.outputEntry(*this, entryRef);
    ASSERT_TRUE(checkAndResetEntry("1 2348 1 0 ver<?>"));
}

} // namespace
