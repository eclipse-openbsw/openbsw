// Copyright 2024 Accenture.

#include "logger/ConsoleEntryOutput.h"

#include <util/StdIoMock.h>
#include <util/format/IPrintfArgumentReader.h>
#include <util/format/StringWriter.h>
#include <util/logger/Logger.h>

#include <gtest/gtest.h>

#include <sstream>

using namespace ::util::format;
using namespace ::util::logger;
using namespace ::logger;

namespace
{
struct ConsoleEntryOutputTest
: ::testing::Test
, IEntryFormatter<uint32_t, uint32_t>
, IPrintfArgumentReader
{
    void formatEntry(
        ::util::stream::IOutputStream& outputStream,
        uint32_t entryIndex,
        uint32_t timestamp,
        ComponentInfo const& componentInfo,
        LevelInfo const& levelInfo,
        char const* str,
        IPrintfArgumentReader& argReader) const override
    {
        StringWriter writer(outputStream);
        writer.printf(
            "%d %d %d %s ",
            entryIndex,
            timestamp,
            componentInfo.getIndex(),
            levelInfo.getName().getString());
        writer.vprintf(str, argReader);
    }

    ParamVariant const* readArgument(ParamDatatype) override
    {
        switch (_argumentIdx++)
        {
            case 0:  _variant._uint32Value = 83743; break;
            case 1:  _variant._charPtrValue = "String"; break;
            default: return nullptr;
        }
        return &_variant;
    }

    ::util::test::StdIoMock stdIo;

    uint32_t _argumentIdx = 0;
    ParamVariant _variant{};
};

TEST_F(ConsoleEntryOutputTest, testAll)
{
    ConsoleEntryOutput<uint32_t, uint32_t> cut(*this);
    ComponentInfo::PlainInfo const componentInfo = {{"Component_Name", {}}};
    LevelInfo::PlainInfo const levelInfo         = {{"Level_Name", {}}, {}};
    cut.outputEntry(
        15,
        15343,
        ComponentInfo(16, &componentInfo),
        LevelInfo(&levelInfo),
        "Format string %d %s",
        *this);
    ASSERT_EQ(
        "15 15343 16 Level_Name Format string 83743 String\r\n",
        std::string(stdIo.out.begin(), stdIo.out.end()));
}

} // namespace
