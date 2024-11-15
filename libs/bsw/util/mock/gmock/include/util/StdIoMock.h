// Copyright 2024 Accenture.

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
