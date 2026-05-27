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
