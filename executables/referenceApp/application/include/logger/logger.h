// Copyright 2024 Accenture.

#pragma once

#include <logger/ComponentMapping.h>

#ifdef BUILD_RUST
#include <util/logger/Logger.h>

DECLARE_LOGGER_COMPONENT(RUST)
#endif

namespace logger
{
void init();
void run();
void flush();

} // namespace logger
