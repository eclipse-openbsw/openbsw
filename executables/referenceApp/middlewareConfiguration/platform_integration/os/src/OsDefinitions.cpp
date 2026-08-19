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

#include <async/AsyncBinding.h>

#include <middleware/os/TaskIdProvider.h>

namespace middleware
{
namespace os
{

uint32_t getProcessId() { return ::async::AsyncBinding::AdapterType::getCurrentTaskContext(); }

} // namespace os
} // namespace middleware
