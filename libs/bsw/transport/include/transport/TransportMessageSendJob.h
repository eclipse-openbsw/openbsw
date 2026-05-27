/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup transport
 */
#pragma once

#include <etl/intrusive_list.h>

namespace transport
{
class TransportMessage;
class ITransportMessageProcessedListener;

class TransportMessageSendJob : public ::etl::bidirectional_link<0>
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
: ::etl::bidirectional_link<0>(), fpTransportMessage(nullptr), fpListener(nullptr)
{}

inline TransportMessageSendJob::TransportMessageSendJob(
    TransportMessage* const pTransportMessage, ITransportMessageProcessedListener* const pListener)
: ::etl::bidirectional_link<0>(), fpTransportMessage(pTransportMessage), fpListener(pListener)
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
