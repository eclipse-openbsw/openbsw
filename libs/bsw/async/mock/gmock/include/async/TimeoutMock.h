// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "async/Async.h"
#include "estd/functional.h"
#include "estd/singleton.h"

#include <gmock/gmock.h>

namespace async
{
class TimeoutMock : public ::estd::singleton<TimeoutMock>
{
public:
    TimeoutMock() : ::estd::singleton<TimeoutMock>(*this) {}

    MOCK_METHOD1(cancel, void(TimeoutType& timeout));
};

} // namespace async
