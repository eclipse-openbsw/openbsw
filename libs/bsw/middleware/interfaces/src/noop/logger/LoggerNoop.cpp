/*******************************************************************************
 *
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************
 */
#include "middleware/logger/Logger.h"

namespace middleware::logger
{

// Default stub for unconfigured builds: discards all log output. Platform
// integrations must supply a real implementation via the logger_impl label_flag.
// NOLINTBEGIN(cert-dcl50-cpp,cppcoreguidelines-pro-type-vararg)
void log(LogLevel const /*level*/, char const* const /*f*/, ...) {}

// NOLINTEND(cert-dcl50-cpp,cppcoreguidelines-pro-type-vararg)

void logBinary(LogLevel const /*level*/, ::etl::span<uint8_t const> const /*data*/) {}

uint32_t getMessageId(Error const /*id*/) { return 0U; }

} // namespace middleware::logger
