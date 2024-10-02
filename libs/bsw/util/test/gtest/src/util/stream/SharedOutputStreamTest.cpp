// Copyright 2024 Accenture.

#include "util/stream/SharedOutputStream.h"

#include "gtest/gtest.h"
#include "util/stream/StringBufferOutputStream.h"

namespace
{
using namespace ::util::stream;

struct TestContinuousUser : public ISharedOutputStream::IContinuousUser
{
    TestContinuousUser() : _endOutputStream(nullptr) {}

    void endContinuousOutput(IOutputStream& stream) override { _endOutputStream = &stream; }

    IOutputStream* checkAndResetEndOutputStream()
    {
        IOutputStream* result = _endOutputStream;
        _endOutputStream      = nullptr;
        return result;
    }

    IOutputStream* _endOutputStream;
};

class SharedOutputStreamTest
: public ::testing::Test
, public TestContinuousUser
{};

TEST_F(SharedOutputStreamTest, testStreamIsReturned)
{
    declare::StringBufferOutputStream<80> stream;
    SharedOutputStream cut(stream);
    ASSERT_EQ(&stream, &cut.startOutput(nullptr));
    ASSERT_EQ(&stream, &cut.startOutput(nullptr));
    cut.endOutput(nullptr);
    ASSERT_EQ(&stream, &cut.startOutput(nullptr));
}

TEST_F(SharedOutputStreamTest, testUserIsObeyed)
{
    TestContinuousUser testUser;
    declare::StringBufferOutputStream<80> stream;
    SharedOutputStream cut(stream);
    ASSERT_EQ(&stream, &cut.startOutput(this));
    cut.endOutput(this);
    cut.startOutput(nullptr);
    ASSERT_EQ(&stream, checkAndResetEndOutputStream());
    ASSERT_EQ(&stream, &cut.startOutput(this));
    cut.endOutput(this);
    cut.releaseContinuousUser(testUser);
    cut.releaseContinuousUser(*this);
    ASSERT_EQ(&stream, checkAndResetEndOutputStream());
}

TEST_F(SharedOutputStreamTest, testUserContinuousIsNotified)
{
    declare::StringBufferOutputStream<80> stream;
    SharedOutputStream cut(stream);
    ASSERT_EQ(&stream, &cut.startOutput(this));
    cut.endOutput(this);
    ASSERT_EQ(nullptr, checkAndResetEndOutputStream());
    ASSERT_EQ(&stream, &cut.startOutput(this));
    ASSERT_EQ(nullptr, checkAndResetEndOutputStream());
    cut.endOutput(this);
}

} // anonymous namespace
