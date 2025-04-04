// Copyright 2024 Accenture.

/**
 * \ingroup transport
 */
#pragma once

#include "transport/ITransportMessageListener.h"
#include "transport/ITransportMessageProvider.h"

namespace transport
{
class TransportMessage;

/**
 * interface for Listeners that provide their own TransportMessage to receive to
 *
 * \see ITransportMessageListener
 * \see ITransportMessageProvider
 * \par
 * A typical TransportMessage listening transaction has the following steps:
 * - a transport layer call getTransportMessage to get a buffer to receive to
 * - once the message is completely received messageReceived gets called
 * - when the transport layer is done with the message it calls
 * releaseTransportMessage and the message may be used again
 */
class ITransportMessageProvidingListener
: public ITransportMessageListener
, public ITransportMessageProvider
{
public:
    ITransportMessageProvidingListener& operator=(ITransportMessageProvidingListener const&)
        = delete;
};

} // namespace transport
