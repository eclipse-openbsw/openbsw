// Copyright 2024 Accenture.

#pragma once

#include "util/stream/BspStubs.h"

#include <estd/singleton.h>
#include <estd/slice.h>

#include <vector>

namespace util
{
namespace test
{
struct StdIoMock : ::estd::singleton<StdIoMock>
{
    StdIoMock() : ::estd::singleton<StdIoMock>(*this) {}

    std::vector<uint8_t> out;
    ::estd::slice<uint8_t const> in;
};

} // namespace test
} // namespace util
