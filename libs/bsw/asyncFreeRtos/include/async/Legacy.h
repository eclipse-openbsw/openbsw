// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_71FFB374_BE25_48D7_A2A7_947AA3BCD696
#define GUARD_71FFB374_BE25_48D7_A2A7_947AA3BCD696

#include "async/Types.h"

namespace common
{
class ITimeoutManager2;
}

namespace loop
{
class looper;
}

namespace async
{
::common::ITimeoutManager2& getTimeoutManager(ContextType context);
::loop::looper& getLooper(ContextType context);

} // namespace async

#endif // GUARD_71FFB374_BE25_48D7_A2A7_947AA3BCD696
