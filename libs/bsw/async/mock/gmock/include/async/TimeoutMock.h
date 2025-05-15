// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "async/Async.h"

#include <etl/singleton_base.h>

#include <gmock/gmock.h>

namespace async
{
class TimeoutMock : public ::etl::singleton_base<TimeoutMock>
{
public:
    TimeoutMock() : ::etl::singleton_base<TimeoutMock>(*this) {}

    MOCK_METHOD(void, cancel, (TimeoutType & timeout));
};

} // namespace async
