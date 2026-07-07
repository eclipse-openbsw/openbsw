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

#include <etl/expected.h>
#include <etl/functional.h>
#include <gmock/gmock.h>

#include "middleware/core/Future.h"

namespace middleware::core::test
{

template<typename ArgType>
class RefApp
{
public:
    MOCK_METHOD(
        void,
        methodCallback,
        ((::etl::expected<::etl::reference_wrapper<ArgType const>, Future::State> const&)));
    MOCK_METHOD(void, eventCallback, (ArgType const&));
};

} // namespace middleware::core::test
