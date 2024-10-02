// Copyright 2024 Accenture.

#ifndef GUARD_43DE2F28_C073_41B7_81DE_F4154115A92D
#define GUARD_43DE2F28_C073_41B7_81DE_F4154115A92D

#include "util/stream/ISharedOutputStream.h"

#include <gmock/gmock.h>

namespace util
{
namespace stream
{
class SharedOutputStreamMock : public ISharedOutputStream
{
public:
    MOCK_METHOD1(startOutput, IOutputStream&(IContinuousUser* user));
    MOCK_METHOD1(endOutput, void(IContinuousUser* user));
    MOCK_METHOD1(releaseContinuousUser, void(IContinuousUser& user));
};

class ContinuousUserMock : public ISharedOutputStream::IContinuousUser
{
public:
    MOCK_METHOD1(endContinuousOutput, void(IOutputStream& stream));
};

} // namespace stream
} // namespace util

#endif /* GUARD_43DE2F28_C073_41B7_81DE_F4154115A92D */
