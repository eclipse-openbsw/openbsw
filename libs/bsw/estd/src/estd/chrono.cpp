// Copyright 2024 Accenture.

#ifndef ESTD_NO_CHRONO

#include "estd/chrono.h"

namespace estd
{
namespace chrono
{
high_resolution_clock::time_point high_resolution_clock::now()
{
    return time_point(duration(static_cast<int64_t>(getSystemTimeNs())));
}

} // namespace chrono
} // namespace estd

#endif // ifndef ESTD_NO_CHRONO
