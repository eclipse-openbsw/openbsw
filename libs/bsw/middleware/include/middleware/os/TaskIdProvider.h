// Copyright 2025 BMW AG

#pragma once

#include <cstdint>

namespace middleware
{
namespace os
{

/**
 * \brief Get the current process/task ID.
 * \details Platform-specific function that returns the identifier of the currently executing
 * process or task. This is used for thread-safety validation to detect cross-thread access
 * violations in proxies and skeletons. The implementation must be provided for each platform
 * integration.
 *
 * \return the current process/task ID
 */
extern uint32_t getProcessId();

} // namespace os
} // namespace middleware
