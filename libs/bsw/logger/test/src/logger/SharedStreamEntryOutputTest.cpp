// Copyright 2024 Accenture.

#include "logger/SharedStreamEntryOutput.h"

#include <util/format/IPrintfArgumentReader.h>
#include <util/format/StringWriter.h>
#include <util/logger/Logger.h>
#include <util/stream/SharedOutputStream.h>
#include <util/stream/StringBufferOutputStream.h>

#include <gtest/gtest.h>

#include <sstream>

using namespace ::util::format;
using namespace ::util::logger;
using namespace ::util::stream;
using namespace ::logger;

namespace
{
class SharedOutputStreamMock : public ISharedOutputStream
{
public:
    SharedOutputStreamMock() = default;

    IOutputStream& startOutput(ISharedOutputStream::IContinuousUser* /*user*/) override
    {
        return _stream;
    }

    void endOutput(ISharedOutputStream::IContinuousUser* /*user*/) override {}

    void releaseContinuousUser(ISharedOutputStream::IContinuousUser& /*user*/) override {}

    StringBufferOutputStream& getStream() { return _stream; }

private:
    declare::StringBufferOutputStream<80> _stream;
};

struct SharedStreamEntryOutputTest
: ::testing::Test
, IEntryFormatter<uint32_t, uint32_t>
, IPrintfArgumentReader
{
    void formatEntry(
        ::util::stream::IOutputStream& stream,
        uint32_t entryIndex,
        uint32_t timestamp,
        ComponentInfo const& componentInfo,
        LevelInfo const& levelInfo,
        char const* str,
        IPrintfArgumentReader& argReader) const override
    {
        StringWriter writer(stream);
        writer.printf(
            "%d %d %d %s ",
            entryIndex,
            timestamp,
            componentInfo.getIndex(),
            levelInfo.getName().getString());
        writer.vprintf(str, argReader);
    }

    ParamVariant const* readArgument(ParamDatatype /*datatype*/) override
    {
        switch (_argumentIdx++)
        {
            case 0:  _variant._uint32Value = 83743; break;
            case 1:  _variant._charPtrValue = "String"; break;
            default: return nullptr;
        }
        return &_variant;
    }

    uint32_t _argumentIdx = 0;
    ParamVariant _variant{};
};

TEST_F(SharedStreamEntryOutputTest, testAll)
{
    SharedOutputStreamMock streamMock;
    SharedStreamEntryOutput<uint32_t, uint32_t> cut(streamMock, *this);
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
        "15 15343 16 Level_Name Format string 83743 String",
        std::string(streamMock.getStream().getString()));
}

} // namespace
