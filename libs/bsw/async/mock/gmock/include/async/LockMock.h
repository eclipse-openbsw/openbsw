// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include <etl/singleton_base.h>

#include <gmock/gmock.h>

namespace async
{
class LockMock : public ::etl::singleton_base<LockMock>
{
public:
    LockMock() : ::etl::singleton_base<LockMock>(*this) {}

    MOCK_METHOD(void, lock, ());
    MOCK_METHOD(void, unlock, ());
};

} // namespace async
