/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
