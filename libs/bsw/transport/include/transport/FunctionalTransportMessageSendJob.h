// Copyright 2024 Accenture.

/**
 * \ingroup transport
 */
#ifndef GUARD_2F882E8D_78A3_437F_AA73_8B4BFE009799
#define GUARD_2F882E8D_78A3_437F_AA73_8B4BFE009799

#include "transport/TransportMessageSendJob.h"

#include <etl/intrusive_forward_list.h>

#include <cstdint>

namespace transport
{
class FunctionalTransportMessageSendJob : public ::etl::forward_link<0>
{
public:
    FunctionalTransportMessageSendJob() = default;

    FunctionalTransportMessageSendJob(FunctionalTransportMessageSendJob const&)            = delete;
    FunctionalTransportMessageSendJob& operator=(FunctionalTransportMessageSendJob const&) = delete;

    TransportMessageSendJob& getTransportMessageSendJob() { return fTransportMessageSendJob; }

    void setTransportMessageProcessedCounter(uint8_t const transportMessageProcessedCounter)
    {
        fTransportMessageProcessedCounter = transportMessageProcessedCounter;
    }

    void decreaseTransportMessageProcessedCounter()
    {
        if (fTransportMessageProcessedCounter > 0U)
        {
            --fTransportMessageProcessedCounter;
        }
    }

    uint8_t getTransportMessageProcessedCounter() const
    {
        return fTransportMessageProcessedCounter;
    }

private:
    TransportMessageSendJob fTransportMessageSendJob;
    uint8_t fTransportMessageProcessedCounter = 0U;
};
} // namespace transport

#endif /*GUARD_2F882E8D_78A3_437F_AA73_8B4BFE009799*/
