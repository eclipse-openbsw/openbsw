// Copyright 2024 Accenture.

#pragma once

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
