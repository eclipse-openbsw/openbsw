// Copyright 2024 Accenture.

#include "util/stream/TaggedSharedOutputStream.h"

#include "util/stream/ISharedOutputStream.h"
#include "util/stream/StringBufferOutputStream.h"

#include <gtest/gtest.h>

#include <sstream>

namespace
{
using namespace ::util::stream;

struct TaggedSharedOutputStreamTestFixture
: public ::testing::Test
, public ISharedOutputStream
, public ISharedOutputStream::IContinuousUser
{
    TaggedSharedOutputStreamTestFixture() : _lockCount(0), _user(nullptr), _endCalled(false) {}

    void init(IOutputStream& stream) { _stream = &stream; }

    IOutputStream& startOutput(ISharedOutputStream::IContinuousUser* user) override
    {
        if (_user && user != _user)
        {
            _user->endContinuousOutput(*_stream);
        }
        _user = user;
        ++_lockCount;
        return *_stream;
    }

    void endOutput(ISharedOutputStream::IContinuousUser* user) override
    {
        if (!user)
        {
            _user = nullptr;
        }
    }

    void releaseContinuousUser(ISharedOutputStream::IContinuousUser& user) override
    {
        if (_user == &user)
        {
            _user = nullptr;
        }
        _endCalled = true;
    }

    void endContinuousOutput(IOutputStream& /* stream */) override {}

    bool hasUser() const { return _user != nullptr; }

    bool checkAndResetEndCalled()
    {
        bool result = _endCalled;
        _endCalled  = false;
        return result;
    }

    uint32_t _lockCount;
    ISharedOutputStream::IContinuousUser* _user;
    IOutputStream* _stream{};
    bool _endCalled;
};

class ContinuousUser : public ISharedOutputStream::IContinuousUser
{
public:
    void endContinuousOutput(IOutputStream& /* stream */) override {}
};

TEST_F(TaggedSharedOutputStreamTestFixture, testPrefixAndSuffixesAreInserted)
{
    {
        declare::StringBufferOutputStream<80> bufferStream;
        init(bufferStream);
        {
            TaggedSharedOutputStream cut(*this, "[START]", "[CRLF]");
            IOutputStream& stream = cut.startOutput(nullptr);
            stream.write('a');
            stream.write('\n');
            stream.write(::estd::make_str("abc\ndef"));
            stream.write(::estd::make_str("AB\nDEF"));
            cut.endOutput(nullptr);
        }
        ASSERT_FALSE(checkAndResetEndCalled());
        ASSERT_FALSE(hasUser());
        ASSERT_EQ(
            "[START]a[CRLF][START]abc[CRLF][START]defAB[CRLF][START]DEF[CRLF]",
            std::string(bufferStream.getString()));
    }
    {
        declare::StringBufferOutputStream<80> bufferStream;
        init(bufferStream);
        TaggedSharedOutputStream cut(*this, "[START]", "[CRLF]");
        IOutputStream& stream = cut.startOutput(nullptr);
        stream.write('\n');
        cut.endOutput(nullptr);
        stream.write('a');
        stream.write(::estd::make_str("abc\ndef"));
        ASSERT_EQ("[START][CRLF]", std::string(bufferStream.getString()));
    }
}

TEST_F(TaggedSharedOutputStreamTestFixture, testNonContinuousStream)
{
    declare::StringBufferOutputStream<80> bufferStream;
    init(bufferStream);
    TaggedSharedOutputStream cut(*this, "[START]", "[CRLF]");
    IOutputStream* stream = &cut.startOutput(this);
    stream->write('a');
    stream->write('b');
    cut.endOutput(this);
    stream = &cut.startOutput(this);
    stream->write('c');
    cut.endOutput(this);
    ContinuousUser otherUser;
    cut.releaseContinuousUser(otherUser);
    cut.releaseContinuousUser(*this);
    ASSERT_EQ("[START]ab[CRLF][START]c[CRLF]", std::string(bufferStream.getString()));
}

TEST_F(TaggedSharedOutputStreamTestFixture, testContinuousStream)
{
    declare::StringBufferOutputStream<80> bufferStream;
    init(bufferStream);
    TaggedSharedOutputStream cut(*this, "[START]", "[CRLF]", true);
    IOutputStream* stream = &cut.startOutput(this);
    stream->write('a');
    stream->write('b');
    cut.endOutput(this);
    stream = &cut.startOutput(this);
    stream->write('c');
    cut.endOutput(this);
    ContinuousUser otherUser;
    stream = &cut.startOutput(&otherUser);
    stream->write('d');
    cut.endOutput(&otherUser);
    stream = &cut.startOutput(this);
    cut.endOutput(this);
    stream = &cut.startOutput(this);
    stream->write('e');
    cut.endOutput(this);
    startOutput(nullptr);
    endOutput(nullptr);
    stream = &cut.startOutput(this);
    stream->write('f');
    cut.releaseContinuousUser(otherUser);
    cut.releaseContinuousUser(*this);
    ASSERT_EQ(
        "[START]abc[CRLF][START]d[CRLF][START]e[CRLF][START]f[CRLF]",
        std::string(bufferStream.getString()));
}

TEST_F(TaggedSharedOutputStreamTestFixture, testEofIsReportedCorrectly)
{
    declare::StringBufferOutputStream<10> bufferStream;
    init(bufferStream);
    TaggedSharedOutputStream cut(*this, "", "[CRLF]");
    IOutputStream& stream = cut.startOutput(nullptr);
    ASSERT_FALSE(stream.isEof());
    stream.write('a');
    stream.write('b');
    stream.write('\n');
    ASSERT_FALSE(stream.isEof());
    stream.write('a');
    ASSERT_TRUE(stream.isEof());
    cut.endOutput(nullptr);
    ASSERT_TRUE(stream.isEof());
}

} // anonymous namespace
