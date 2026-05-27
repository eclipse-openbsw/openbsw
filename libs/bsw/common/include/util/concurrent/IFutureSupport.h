/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
