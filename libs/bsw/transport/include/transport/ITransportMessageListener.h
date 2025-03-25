// Copyright 2024 Accenture.

/**
 * \ingroup transport
 */
#pragma once

#include <platform/estdint.h>

namespace transport
{
class TransportMessage;
class ITransportMessageProcessedListener;

/**
 * Interface for a class that receives TransportMessages
 *
 * This class does not provide any buffers to receive the TransportMessage to.
 * It just wants to be notified when the TransportMessage is received.
 */
class ITransportMessageListener
{
    ITransportMessageListener& operator=(ITransportMessageListener const&);

public:
    /**
     * Status indicating the result of receiving a TransportMessage
     */
    enum class ReceiveResult : uint8_t
    {
        /** no error occurred during the receiving */
        RECEIVED_NO_ERROR,
        /** an error occurred during the receiving */
        RECEIVED_ERROR
    };

    /**
     * callback when a TransportMessage has been received completely
     * \param sourceBusId            id of bus message is received from
     * \param transportMessage       TransportMessage that was received
     * \param pNotificationListener  listener that wants to be notified when
     * the transportMessage has been processed. May be NULL which implies that
     * no notification is required.
     * \return  the immediate result of receiving the message
     */
    virtual ReceiveResult messageReceived(
        uint8_t sourceBusId,
        TransportMessage& transportMessage,
        ITransportMessageProcessedListener* pNotificationListener)
        = 0;
};

} // namespace transport

