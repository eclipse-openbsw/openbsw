// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */
#pragma once
#include "gmock/gmock.h"
#include "util/timeout/AbstractTimeout.h"

namespace common
{
class AbstractTimeoutMock : public AbstractTimeout
{
public:
    MOCK_METHOD(void, expired, (TimeoutExpiredActions));
};
} // namespace common
