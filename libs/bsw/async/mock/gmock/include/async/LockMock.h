// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_8926F302_BFFF_44FF_92EE_977F85994019
#define GUARD_8926F302_BFFF_44FF_92EE_977F85994019

#include "estd/singleton.h"

#include <gmock/gmock.h>

namespace async
{
class LockMock : public ::estd::singleton<LockMock>
{
public:
    LockMock() : ::estd::singleton<LockMock>(*this) {}

    MOCK_METHOD0(lock, void());
    MOCK_METHOD0(unlock, void());
};

} // namespace async

#endif // GUARD_8926F302_BFFF_44FF_92EE_977F85994019
