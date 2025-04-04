// Copyright 2024 Accenture.

/**
 * \ingroup transport
 */
#pragma once

#include <estd/forward_list.h>

namespace transport
{
class TransportMessage;
class ITransportMessageProcessedListener;

class TransportMessageSendJob : public ::estd::forward_list_node<TransportMessageSendJob>
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
: ::estd::forward_list_node<TransportMessageSendJob>()
, fpTransportMessage(nullptr)
, fpListener(nullptr)
{}

inline TransportMessageSendJob::TransportMessageSendJob(
    TransportMessage* const pTransportMessage, ITransportMessageProcessedListener* const pListener)
: ::estd::forward_list_node<TransportMessageSendJob>()
, fpTransportMessage(pTransportMessage)
, fpListener(pListener)
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
