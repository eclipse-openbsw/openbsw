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

#include "util/stream/BspStubs.h"

#include <etl/singleton_base.h>
#include <etl/span.h>

#include <vector>

namespace util
{
namespace test
{
struct StdIoMock : ::etl::singleton_base<StdIoMock>
{
    StdIoMock() : ::etl::singleton_base<StdIoMock>(*this) {}

    std::vector<uint8_t> out;
    ::etl::span<uint8_t const> in;
};

} // namespace test
} // namespace util
