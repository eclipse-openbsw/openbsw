// Copyright 2024 Accenture.

/**
 * \ingroup transport
 */
#ifndef GUARD_1D3052EC_A91A_45FE_89F7_9767C8B0EC00
#define GUARD_1D3052EC_A91A_45FE_89F7_9767C8B0EC00

#include <etl/intrusive_forward_list.h>

namespace transport
{
class TransportMessage;
class ITransportMessageProcessedListener;

class TransportMessageSendJob : public ::etl::forward_link<0>
{
public:
    TransportMessageSendJob();

    TransportMessageSendJob(TransportMessageSendJob const&)            = delete;
    TransportMessageSendJob& operator=(TransportMessageSendJob const&) = delete;

    TransportMessageSendJob(
        TransportMessage* pTransportMessage, ITransportMessageProcessedListener* pListener);

    void setTransportMessage(TransportMessage* pTransportMessage);

    TransportMessage* getTransportMessage();

    void setTransportMessageProcessedListener(ITransportMessageProcessedListener* pListener);

    ITransportMessageProcessedListener* getTransportMessageProcessedListener();

private:
    TransportMessage* fpTransportMessage;
    ITransportMessageProcessedListener* fpListener;
};

/*
 *
 * inline implementation
 *
 */

inline TransportMessageSendJob::TransportMessageSendJob()
: ::etl::forward_link<0>(), fpTransportMessage(nullptr), fpListener(nullptr)
{}

inline TransportMessageSendJob::TransportMessageSendJob(
    TransportMessage* const pTransportMessage, ITransportMessageProcessedListener* const pListener)
: ::etl::forward_link<0>(), fpTransportMessage(pTransportMessage), fpListener(pListener)
{}

inline void TransportMessageSendJob::setTransportMessage(TransportMessage* const pTransportMessage)
{
    fpTransportMessage = pTransportMessage;
}

inline TransportMessage* TransportMessageSendJob::getTransportMessage()
{
    return fpTransportMessage;
}

inline void TransportMessageSendJob::setTransportMessageProcessedListener(
    ITransportMessageProcessedListener* const pListener)
{
    fpListener = pListener;
}

inline ITransportMessageProcessedListener*
TransportMessageSendJob::getTransportMessageProcessedListener()
{
    return fpListener;
}

} // namespace transport

#endif // GUARD_1D3052EC_A91A_45FE_89F7_9767C8B0EC00
