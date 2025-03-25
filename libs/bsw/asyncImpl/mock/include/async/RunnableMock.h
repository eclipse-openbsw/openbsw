// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "async/IRunnable.h"

#include <gmock/gmock.h>

namespace async
{
class RunnableMock : public IRunnable
{
public:
    MOCK_METHOD0(execute, void());
};

} // namespace async

