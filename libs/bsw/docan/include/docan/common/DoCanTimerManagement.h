// Copyright 2024 Accenture.

#pragma once

#include <platform/estdint.h>

#include <limits>

namespace docan
{
namespace timermanagement
{
inline bool less(uint32_t const firstTime, uint32_t const secondTime)
{
    // Separated out as it's used twice in both MessageReceiver and MessageTransmitter
    //
    // "firstTime" and "secondTime" must be unsigned integers for this calculation to work! We
    // depend on wraparound to produce correct behavior in this code (don't worry, wraparound is
    // well-defined in the C++ standard, it's signed integer overflow which is UB)
    //
    // The way this works is that if firstTime - secondTime is larger than INT32_MAX then that
    // means secondTime > firstTime. If firstTime - secondTime is
    // smaller than INT32_MAX then that means secondTime <= firstTime.
    //
    // The caveat to this code is that for correct operation we must guarantee that the integrating
    // system must always wait less than INT32_MAX microseconds in between time updates. If it does
    // wait INT32_MAX microseconds or more in between time updates, the subsequent subtraction of
    // firstTime - secondTime will almost certainly produce a result less than INT32_MAX,
    // meaning the system will believe the given container is not yet ready to send, when actually
    // it most certainly is. However, INT32_MAX microseconds is 35 minutes, 47 seconds, so if the
    // system has waited that long in between time updates then something's obviously very wrong.
    return (firstTime - secondTime) > static_cast<uint32_t>(::std::numeric_limits<int32_t>::max());
}
} // namespace timermanagement
} // namespace docan

