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

#include "middleware/shm/Config.h"

namespace middleware::shm
{
MemoryLayout& getMemoryLayout()
{
    static MemoryLayout layout;
    return layout;
}
} // namespace middleware::shm
