// Copyright 2024 Accenture.

#ifndef GUARD_4E0CAF56_BD8A_4C9C_98F4_9C8DBF1A0BB7
#define GUARD_4E0CAF56_BD8A_4C9C_98F4_9C8DBF1A0BB7

namespace os
{
/**
 * Class providing interface for synchronization between the main thread and the worker thread
 *
 *
 */
class IFutureSupport
{
public:
    IFutureSupport(IFutureSupport const&)            = delete;
    IFutureSupport& operator=(IFutureSupport const&) = delete;
    IFutureSupport()                                 = default;

    virtual void wait()              = 0;
    virtual void notify()            = 0;
    virtual void assertTaskContext() = 0;
    virtual bool verifyTaskContext() = 0;
};

} /* namespace os */

#endif /* GUARD_4E0CAF56_BD8A_4C9C_98F4_9C8DBF1A0BB7 */
