/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#include <cstdint>

#include <unistd.h>

#include <middleware/os/TaskIdProvider.h>

namespace middleware
{
namespace os
{

uint32_t getProcessId()
{
    pid_t const tid = gettid();
    static_assert(sizeof tid <= sizeof(uint32_t), "Size of tid is bigger than sizeof(uint32_t)");
    return static_cast<uint32_t>(tid);
}

} // namespace os
} // namespace middleware
