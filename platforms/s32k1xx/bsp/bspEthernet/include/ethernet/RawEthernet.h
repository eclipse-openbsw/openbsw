// Copyright 2025 Accenture.

#pragma once

#ifdef __cplusplus

#include "etl/chrono.h"
#include "etl/delegate.h"
#include "etl/span.h"

namespace ethernet
{

typedef ::etl::delegate<void(
    ::etl::span<uint8_t const>, // data
    ::etl::chrono::nanoseconds  // systemTicks
    )>
    DataSentCallback;

} // namespace ethernet

extern "C"
{
#endif // #ifdef __cplusplus

uint8_t ethernet_write(uint8_t data[], uint32_t length);

#ifdef __cplusplus
} // extern "C"
#endif
