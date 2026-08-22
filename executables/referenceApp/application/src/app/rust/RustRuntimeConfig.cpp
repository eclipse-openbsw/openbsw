/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "app/rust/RustRuntimeConfig.h"

#include "app/rust/RustRuntime.h"

#include <async/Config.h>

namespace
{
::app::RustRuntime demoRuntime{TASK_DEMO};
::app::RustRuntime backgroundRuntime{TASK_BACKGROUND};
} // namespace

namespace app
{
void initRustRuntimes()
{
    demoRuntime.init();
    backgroundRuntime.init();
}

} // namespace app
