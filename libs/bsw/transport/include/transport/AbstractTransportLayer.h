// Copyright 2024 Accenture.

/**
 * \ingroup transport
 */
#ifndef GUARD_DB3117F1_61DE_4245_8596_EE8DA29DD210
#define GUARD_DB3117F1_61DE_4245_8596_EE8DA29DD210

#include "transport/ITransportMessageProvidingListener.h"

#include <estd/forward_list.h>
#include <estd/functional.h>

namespace transport
{
/**
 * Base class for transport layers providing some common functionality.
 *
 * \par Usage
 */
class AbstractTransportLayer : public ::estd::forward_list_node<AbstractTransportLayer>
{
public:
    /** Delegate for asynchronous notification when shutdown is done. */
    using ShutdownDelegate = ::estd::function<void(AbstractTransportLayer&)>;

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
     * ::transport::ITransportMessageProvider has been set using
     * setTransportMessageProvider(). To actually get access to the received
     * messages set an ITransportMessageListener using
     * setTransportMessageListener().
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

    /**
     * \param pProvider Pointer to ITransportMessageProvider
     */
    void setTransportMessageProvider(ITransportMessageProvider* pProvider);

    /**
     * \param pListener Pointer to ITransportMessageListener
     */
    void setTransportMessageListener(ITransportMessageListener* pListener);

protected:
    /**
     * Provides access to ITransportMessageProvidingListener for derived
     * classes.
     */
    ITransportMessageProvidingListener& getProvidingListenerHelper();

    /**
     * This function may be used by all subclasses as a default initialization
     * of an instance variable of type ShutdownDelegate.
     */
    static void shutdownCompleteDummy(AbstractTransportLayer&);

private:
    class TransportMessageProvidingListenerHelper : public ITransportMessageProvidingListener
    {
    public:
        explicit TransportMessageProvidingListenerHelper(uint8_t busId);

        /**
         * \param pProvider Pointer to ITransportMessageProvider
         */
        void setTransportMessageProvider(ITransportMessageProvider* pProvider);

        /**
         * \param pListener Pointer to ITransportMessageListener
         */
        void setTransportMessageListener(ITransportMessageListener* pListener);

        /**
         * \see ITransportMessageProvidingListener::getTransportMessage()
         */
        ITransportMessageProvidingListener::ErrorCode getTransportMessage(
            uint8_t srcBusId,
            uint16_t sourceId,
            uint16_t targetId,
            uint16_t size,
            ::estd::slice<uint8_t const> const& peek,
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

    private:
        uint8_t fBusId;
        ITransportMessageProvider* fpMessageProvider;
        ITransportMessageListener* fpMessageListener;
    };

    uint8_t fBusId;
    TransportMessageProvidingListenerHelper fProvidingListenerHelper;
};

/*
 *
 * inline implementation
 *
 */

inline uint8_t AbstractTransportLayer::getBusId() const { return fBusId; }

inline void
AbstractTransportLayer::setTransportMessageProvider(ITransportMessageProvider* const pProvider)
{
    fProvidingListenerHelper.setTransportMessageProvider(pProvider);
}

inline void
AbstractTransportLayer::setTransportMessageListener(ITransportMessageListener* const pListener)
{
    fProvidingListenerHelper.setTransportMessageListener(pListener);
}

// protected
inline ITransportMessageProvidingListener& AbstractTransportLayer::getProvidingListenerHelper()
{
    return fProvidingListenerHelper;
}

// static
inline void AbstractTransportLayer::shutdownCompleteDummy(AbstractTransportLayer&) {}

inline void
AbstractTransportLayer::TransportMessageProvidingListenerHelper::setTransportMessageProvider(
    ITransportMessageProvider* const pProvider)
{
    fpMessageProvider = pProvider;
}

inline void
AbstractTransportLayer::TransportMessageProvidingListenerHelper::setTransportMessageListener(
    ITransportMessageListener* const pListener)
{
    fpMessageListener = pListener;
}

} // namespace transport

#endif /*GUARD_DB3117F1_61DE_4245_8596_EE8DA29DD210*/
