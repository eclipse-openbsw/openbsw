// Copyright 2025 Accenture.

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
