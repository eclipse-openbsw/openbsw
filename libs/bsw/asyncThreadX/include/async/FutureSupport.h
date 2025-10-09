// Copyright 2025 Accenture.

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
