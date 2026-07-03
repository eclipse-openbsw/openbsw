/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#pragma once

namespace middleware::shm
{

/**
 * Constructs the MemoryLayout in-place via etl::typed_storage::create().
 * Must be called once before any cluster connection is initialized.
 */
void createMemoryLayout();

} // namespace middleware::shm
