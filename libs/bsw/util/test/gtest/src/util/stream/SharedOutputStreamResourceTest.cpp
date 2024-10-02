// Copyright 2024 Accenture.

#include "util/stream/SharedOutputStreamResource.h"

#include "util/stream/SharedOutputStream.h"
#include "util/stream/StringBufferOutputStream.h"

#include <gtest/gtest.h>

using namespace ::util::stream;

struct SharedOutputStreamResourceTestFixture
: public ::testing::Test
, public ISharedOutputStream
, public ISharedOutputStream::IContinuousUser
{
    SharedOutputStreamResourceTestFixture() : _lockCalled(false), _lockCount(0) {}

    void lock()
    {
        _lockCalled = true;
        ++_lockCount;
    }

    void unlock() { --_lockCount; }

    uint32_t getLockCount() const { return _lockCount; }

    IOutputStream& startOutput(ISharedOutputStream::IContinuousUser* user) override
    {
        _user = user;
        return _stream;
    }

    void endOutput(ISharedOutputStream::IContinuousUser* user) override { _user = user; }

    void releaseContinuousUser(ISharedOutputStream::IContinuousUser& user) override
    {
        _user = &user;
    }

    void endContinuousOutput(IOutputStream& /* stream */) override {}

    bool checkAndResetLockCalled()
    {
        bool result = _lockCalled;
        _lockCalled = false;
        return result;
    }

    bool checkAndResetUser()
    {
        bool result = _user == this;
        _user       = nullptr;
        return result;
    }

    IOutputStream& getStream() { return _stream; }

    bool _lockCalled;
    uint32_t _lockCount;
    declare::StringBufferOutputStream<80> _stream;
    ISharedOutputStream::IContinuousUser* _user{};
};

TEST_F(SharedOutputStreamResourceTestFixture, testResourceIsAllocated)
{
    {
        SharedOutputStreamResource<SharedOutputStreamResourceTestFixture> cut(*this, *this);
        ASSERT_EQ(0U, getLockCount());
        ASSERT_EQ(&getStream(), &cut.startOutput(this));
        ASSERT_TRUE(checkAndResetUser());
        ASSERT_EQ(1U, getLockCount());
        cut.endOutput(this);
        ASSERT_TRUE(checkAndResetUser());
        ASSERT_EQ(0U, getLockCount());
        cut.releaseContinuousUser(*this);
        ASSERT_TRUE(checkAndResetLockCalled());
        ASSERT_TRUE(checkAndResetUser());
        ASSERT_EQ(0U, getLockCount());
    }
}
