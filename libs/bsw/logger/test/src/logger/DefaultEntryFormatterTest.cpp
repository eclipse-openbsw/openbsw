// Copyright 2024 Accenture.

#include "logger/DefaultEntryFormatter.h"

#include "logger/DefaultLoggerTime.h"

#include <util/stream/StringBufferOutputStream.h>

#include <gtest/gtest.h>

using namespace logger;
using namespace ::util::logger;

struct DefaultEntryFormatterTest
: ::testing::Test
, ILoggerTime<uint32_t>
, util::format::IPrintfArgumentReader
{
    uint32_t getTimestamp() const override { return 0x3934; }

    void formatTimestamp(
        util::stream::IOutputStream& outputStream, uint32_t const& timestamp) const override
    {
        ASSERT_EQ(0x3934UL, timestamp);
        outputStream.write(::estd::make_str("0x003934"));
    }

    util::format::ParamVariant const* readArgument(util::format::ParamDatatype) override
    {
        switch (_argumentIdx++)
        {
            case 0:  _variant._uint32Value = 83743; break;
            case 1:  _variant._charPtrValue = "String"; break;
            default: return nullptr;
        }
        return &_variant;
    }

    uint8_t _argumentIdx = 0;
    util::format::ParamVariant _variant{};
};

TEST_F(DefaultEntryFormatterTest, testAll)
{
    DefaultEntryFormatter<> cut(*this);
    util::stream::declare::StringBufferOutputStream<200> outputStream;
    ComponentInfo::PlainInfo const componentInfo = {{"Component_Name", {util::format::Color::RED}}};
    LevelInfo::PlainInfo const levelInfo = {{"Level_Name", {util::format::Color::BLUE}}, {}};
    cut.formatEntry(
        outputStream,
        15,
        0x3934,
        ComponentInfo(15, &componentInfo),
        LevelInfo(&levelInfo),
        "Format string %d %s",
        *this);
    ASSERT_EQ(
        "0x003934: \x1b[31mComponent_Name\x1b[0m: \x1b[34mLevel_Name\x1b[0m: Format string 83743 "
        "String",
        std::string(outputStream.getString()));
}
