// Copyright 2024 Accenture.

#ifndef GUARD_40A35A7B_385D_4C52_A692_27C3350DEC91
#define GUARD_40A35A7B_385D_4C52_A692_27C3350DEC91

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

#endif // GUARD_40A35A7B_385D_4C52_A692_27C3350DEC91
