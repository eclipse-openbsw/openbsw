// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "async/Async.h"

#include <gmock/gmock.h>

namespace async
{
class RunnableMock : public RunnableType
{
public:
    MOCK_METHOD(void, execute, (), (override));
};

} // namespace async
