// Copyright 2024 Accenture.

#pragma once

#include "util/stream/ISharedOutputStream.h"

#include <gmock/gmock.h>

namespace util
{
namespace stream
{
class SharedOutputStreamMock : public ISharedOutputStream
{
public:
    MOCK_METHOD(IOutputStream&, startOutput, (IContinuousUser * user), (override);
    MOCK_METHOD(void, endOutput, (IContinuousUser * user), (override));
    MOCK_METHOD(void, releaseContinuousUser, (IContinuousUser & user), (override));
};

class ContinuousUserMock : public ISharedOutputStream::IContinuousUser
{
public:
    MOCK_METHOD(void, endContinuousOutput, (IOutputStream & stream), (override));
};

} // namespace stream
} // namespace util
