// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_29E42A7E_77FA_4C21_BDC2_AE45965FFE05
#define GUARD_29E42A7E_77FA_4C21_BDC2_AE45965FFE05

#include "async/ContextHookMock.h"
#include "async/FreeRtosAdapter.h"
#include "async/TickHookMock.h"

namespace async
{
struct AsyncBinding
{
    static size_t const TASK_COUNT                 = 3U;
    static TickType_t const WAIT_EVENTS_TICK_COUNT = 100U;

    using AdapterType     = FreeRtosAdapter<AsyncBinding>;
    using ContextHookType = ContextHookMock;
    using TickHookType    = TickHookMock;
};

using AsyncBindingType = AsyncBinding;

} // namespace async

#endif // GUARD_29E42A7E_77FA_4C21_BDC2_AE45965FFE05
