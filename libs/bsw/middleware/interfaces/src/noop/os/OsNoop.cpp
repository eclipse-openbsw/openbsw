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
#include "middleware/os/TaskIdProvider.h"

namespace middleware::os
{

// Default stub for unconfigured builds. Platform integrations must supply a
// real implementation via the os_impl label_flag.
uint32_t getProcessId() { return 0U; }

} // namespace middleware::os
