/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "app/app.h"

#include "reset/softwareSystemReset.h"

#include <etl/error_handler.h>
#include <etl/platform.h>
#include <etl/print.h>

#include <cstdint>

/**
 * Reports a failed ETL_ASSERT and resets the ECU.
 *
 * etl_profile.h defines ETL_MINIMAL_ERRORS, so what(), file_name() and line_number() of the
 * exception carry no information. The return address is printed instead, which resolves back
 * to the ETL_ASSERT call site with addr2line against the ELF of the same build.
 *
 * The reset is explicit rather than left to the watchdog, because the watchdog is serviced
 * from TASK_SAFETY, which keeps running when a lower priority task stops. See #575.
 */
ETL_NORETURN void etl_assert_function(etl::exception const&)
{
    etl::print(
        "ETL_ASSERT failed at {:#x}\r\n", reinterpret_cast<uintptr_t>(__builtin_return_address(0)));
    softwareSystemReset();
}

void app_main()
{
    etl::set_assert_function(etl_assert_function);
    ::app::run();
}
