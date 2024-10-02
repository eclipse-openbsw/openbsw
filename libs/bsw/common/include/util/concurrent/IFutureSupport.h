// Copyright 2024 Accenture.

#ifndef GUARD_4E0CAF56_BD8A_4C9C_98F4_9C8DBF1A0BB7
#define GUARD_4E0CAF56_BD8A_4C9C_98F4_9C8DBF1A0BB7

#include <estd/uncopyable.h>

namespace os
{

class IFutureSupport
{
    UNCOPYABLE(IFutureSupport);

public:
    IFutureSupport() {}

    virtual void wait()              = 0;
    virtual void notify()            = 0;
    virtual void assertTaskContext() = 0;
    virtual bool verifyTaskContext() = 0;
};

} /* namespace os */

#endif /* GUARD_4E0CAF56_BD8A_4C9C_98F4_9C8DBF1A0BB7 */
