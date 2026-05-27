/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

namespace lwiputils
{
/** Assert this is running in the correct task context
 *
 * This function gets called at the beginning of various Socket methods, e.g. `send()`,
 * `connect()`, etc. to ensure, those methods are executed in the correct task context.
 *
 * \note
 * Implementing this function is up to project integration code!
 *
 * \code{.cpp}
 *      // example implementation
 *      TaskType actualTask;
 *      GetTaskID(&actualTask);
 *      ETL_ASSERT(actualTask == task_tcpip, ETL_ERROR_GENERIC("..."));
 * \endcode
 */
void TASK_ASSERT_HOOK();

} // namespace lwiputils
