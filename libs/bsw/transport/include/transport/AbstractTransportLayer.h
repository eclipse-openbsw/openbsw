// Copyright 2024 Accenture.

/**
 * \ingroup transport
 */
#pragma once

#include "transport/ITransportMessageProvidingListener.h"

#include <etl/delegate.h>
#include <etl/intrusive_list.h>

namespace transport
{
/**
 * Base class for transport layers providing some common functionality.
 *
 * \par Usage
 */
class AbstractTransportLayer : public ::etl::bidirectional_link<0>
{
public:
    /** Delegate for asynchronous notification when shutdown is done. */
    using ShutdownDelegate = ::etl::delegate<void(AbstractTransportLayer&)>;

    /** Return value of function shutdown in case shutdown was performed
     * synchronously. */
    static bool const SYNC_SHUTDOWN_COMPLETE = true;

    /**
     * All AbstractTransportLayer related error codes
     */
    enum class ErrorCode : uint8_t
    {
        /** everything ok */
        TP_OK,
        /** sending a TransportMessage failed */
        TP_SEND_FAIL,
        /** send queue of AbstractTransportLayer is full */
        TP_QUEUE_FULL,
        /** the message to be sent is not complete */
        TP_MESSAGE_INCOMPLETE,
        /** this message is currently already being sent */
        TP_MESSAGE_ALREADY_IN_PROGRESS,
        /** a general error occurred during an operation */
        TP_GENERAL_ERROR
    };

    /**
     * Constructor
     * \param  busId The AbstractTransportLayer's bus id
     */
    explicit AbstractTransportLayer(uint8_t busId);

    /**
     * Initializes a transport layer instance. After completion of init()
     * the instance is in a state where receiving and sending messages
     * is possible.
     *
     * \note
     * Receiving messages is only possible if an instance of
     * ::transport::ITransportMessageProvider has been registered via
     * fProvidingListenerHelper. Likewise, to access received messages, an
     * ITransportMessageListener has to be registered on the helper.
     *
     * \return  result of initialization
     *          - TP_OK: successfully initialized
     *          - TP_GENERAL_ERROR: error during initialization         *
     */

    virtual ErrorCode init();

    /**
     * Shuts down a transport layer instance. After completion of shutdown
     * no sending and receiving of messages is possible.
     * \param  ShutdownDelegate  Delegate that will be called in case shutdown
     * is completed asynchronously. \return
     *  - true if shutdown is complete
     *  - false if shutdown will be done asynchronously.
     * \note
     * In case shutdown is completed immediately, SYNC_SHUTDOWN_COMPLETE will be
     * returned an ShutdownDelegate will not be called to avoid recursion.
     */

    virtual bool shutdown(ShutdownDelegate);

    /**
     * Sends a TransportMessage
     * \param  transportMessage      TransportMessage to send
     * \param  pNotificationListener ITransportMessageProcessedListener that
     * has to be notified when the transportMessage has been sent. If this
     * argument is NULL no notification is required.
     *
     * \return Result of send operation.
     *         - TP_OK: The transportMessage has been successfully transferred
     * to the appropriate protocol stack and sending is in progress.
     */
    virtual ErrorCode send(
        TransportMessage& transportMessage,
        ITransportMessageProcessedListener* pNotificationListener)
        = 0;

    /**
     * Returns this AbstractTransportLayer's bus id
     */
    uint8_t getBusId() const;

private:
    uint8_t fBusId;

public:
    class TransportMessageProvidingListenerHelper : public ITransportMessageProvidingListener
    {
    public:
        explicit TransportMessageProvidingListenerHelper(uint8_t busId);

        /**
         * \see ITransportMessageProvidingListener::getTransportMessage()
         */
        ITransportMessageProvidingListener::ErrorCode getTransportMessage(
            uint8_t srcBusId,
            uint16_t sourceId,
            uint16_t targetId,
            uint16_t size,
            ::etl::span<uint8_t const> const& peek,
            TransportMessage*& pTransportMessage) override;

        /**
         * \see ITransportMessageProvidingListener::releaseTransportMessage()
         */
        void releaseTransportMessage(TransportMessage& transportMessage) override;

        /**
         * \see ITransportMessageProvidingListener::messageReceived()
         */
        ReceiveResult messageReceived(
            uint8_t sourceBusId,
            TransportMessage& transportMessage,
            ITransportMessageProcessedListener* pNotificationListener) override;

        void dump() override;

        ITransportMessageProvider* fpMessageProvider;
        ITransportMessageListener* fpMessageListener;

    private:
        uint8_t fBusId;
    };

    TransportMessageProvidingListenerHelper fProvidingListenerHelper;

    /**
     * This function may be used by all subclasses as a default initialization
     * of an instance variable of type ShutdownDelegate.
     */
    static void shutdownCompleteDummy(AbstractTransportLayer&);
};

/*
 *
 * inline implementation
 *
 */

inline uint8_t AbstractTransportLayer::getBusId() const { return fBusId; }

// static
inline void AbstractTransportLayer::shutdownCompleteDummy(AbstractTransportLayer&) {}

} // namespace transport
