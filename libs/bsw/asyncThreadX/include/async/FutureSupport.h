/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup async
 */
#pragma once

#include "tx_api.h"

#include <async/Types.h>
#include <etl/string.h>
#include <util/concurrent/IFutureSupport.h>

namespace async
{
class FutureSupport : public ::os::IFutureSupport
{
public:
    explicit FutureSupport(ContextType context);

    void wait() override;
    void notify() override;
    void assertTaskContext() override;
    bool verifyTaskContext() override;

private:
    ContextType _context;
    TX_EVENT_FLAGS_GROUP _eventObject;
    ::etl::string<10> _eventName;
};

} // namespace async
