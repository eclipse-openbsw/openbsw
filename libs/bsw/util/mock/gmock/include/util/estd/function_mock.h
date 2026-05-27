/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <etl/delegate.h>

#include <gmock/gmock.h>

namespace estd
{

template<typename>
struct function_mock;

template<typename R, typename... Ps>
struct function_mock<R(Ps...)>
{
    using self_type = function_mock<R(Ps...)>;

    MOCK_METHOD(void, callee, ());

    operator ::etl::delegate<R(Ps...)>()
    {
        return ::etl::delegate<R(Ps...)>::template create<self_type, &self_type::callee>(*this);
    }
};

} // namespace estd
