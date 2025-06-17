// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "estd/singleton.h"

#include <gmock/gmock.h>

namespace async
{
class LockMock : public ::estd::singleton<LockMock>
{
public:
    LockMock() : ::estd::singleton<LockMock>(*this) {}

    MOCK_METHOD(void, lock, ());
    MOCK_METHOD(void, unlock, ());
};

} // namespace async
