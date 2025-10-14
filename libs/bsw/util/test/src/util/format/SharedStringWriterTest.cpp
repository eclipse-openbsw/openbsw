// Copyright 2024 Accenture.

#include "util/format/SharedStringWriter.h"

#include "util/stream/StringBufferOutputStream.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::util::format;
using namespace ::util::stream;

struct SharedStringWriterTestFixture
: public ::testing::Test
, public ISharedOutputStream
{
public:
    SharedStringWriterTestFixture() : _started(false), _stream(nullptr) {}

    IOutputStream& startOutput(IContinuousUser*) override
    {
        _started = true;
        return *_stream;
    }

    void endOutput(IContinuousUser*) override { _started = false; }

    void releaseContinuousUser(IContinuousUser&) override {}

    bool _started;
    IOutputStream* _stream;
};
} // anonymous namespace

TEST_F(SharedStringWriterTestFixture, testMixedUsage)
{
    declare::StringBufferOutputStream<40> stream;
    _stream = &stream;

    {
        SharedStringWriter cut(*this);
        ASSERT_TRUE(_started);
        cut.printf("Test output.");
    }
    ASSERT_FALSE(_started);
    ASSERT_EQ(std::string(stream.getString()), "Test output.");
}
