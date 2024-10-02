// Copyright 2024 Accenture.

#include "async/AsyncBinding.h"

namespace async
{
using AdapterType = AsyncBindingType::AdapterType;

#ifndef ASYNC_TIMEOUTMANAGER2_DISABLE
::common::ITimeoutManager2& getTimeoutManager(ContextType const context)
{
    return AdapterType::getTimeoutManager(context);
}
#endif // ASYNC_TIMEOUTMANAGER2_DISABLE

#ifndef ASYNC_LOOPER_DISABLE
::loop::looper& getLooper(ContextType const context) { return AdapterType::getLooper(context); }
#endif // ASYNC_LOOPER_DISABLE

} // namespace async
