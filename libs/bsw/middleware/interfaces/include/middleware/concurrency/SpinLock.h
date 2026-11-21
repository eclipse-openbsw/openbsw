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

namespace middleware::concurrency
{

// Since spin lock optimization are hardware dependent, platform integration shall provide
// spinLockOptimization() definition. Otherwise, a link-error is thrown.
void spinLockOptimization();

} // namespace middleware::concurrency
