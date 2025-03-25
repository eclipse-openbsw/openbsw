// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "async/Async.h"

#include <gmock/gmock.h>

namespace async
{
class TimeoutMock : public TimeoutType
{
public:
    MOCK_METHOD1(cancel, void(TimeoutType&));
};
} // namespace async
