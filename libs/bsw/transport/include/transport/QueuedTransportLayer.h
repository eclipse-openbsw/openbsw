// Copyright 2024 Accenture.

/**
 * \ingroup transport
 */
#ifndef GUARD_FEFB28C8_E743_4B4D_B771_AA61385C992E
#define GUARD_FEFB28C8_E743_4B4D_B771_AA61385C992E

#include "transport/AbstractTransportLayer.h"
#include "transport/ITransportMessageProcessedListener.h"
#include "transport/TransportMessage.h"
#include "transport/TransportMessageSendJob.h"

#include <estd/forward_list.h>
#include <estd/object_pool.h>
#include <estd/uncopyable.h>

#include <cstddef>

namespace transport
{
/**
 * Adds a queue to an AbstractTransportLayer.
 *
 * \reentrancy
 * This class is assumed to be used in a non preemptive context! Otherwise
 * send and transportMessageProcessed must be protected by a resource.
 */
class QueuedTransportLayer
: public AbstractTransportLayer
, public ITransportMessageProcessedListener
{
    using SendJobPool = ::estd::object_pool<TransportMessageSendJob>;

public:
    /**
     * Constructor
     * \param transportLayer AbstractTransportLayer to add queuing to.
     */
    QueuedTransportLayer(AbstractTransportLayer& transportLayer);

    QueuedTransportLayer(QueuedTransportLayer const&)            = delete;
    QueuedTransportLayer& operator=(QueuedTransportLayer const&) = delete;

    /**
     * \see AbstractTransportLayer::init()
     */
    AbstractTransportLayer::ErrorCode init() override;

    /**
     * \see AbstractTransportLayer::shutdown()
     */
    bool shutdown(ShutdownDelegate delegate) override;

    /**
     * \see AbstractTransportLayer::send()
     */
    ErrorCode send(
        TransportMessage& transportMessage,
        ITransportMessageProcessedListener* pNotificationListener) override;

    /**
     * \see ITransportMessageProcessedListener::transportMessageProcessed()
     */
    void
    transportMessageProcessed(TransportMessage& transportMessage, ProcessingResult result) override;

    size_t numberOfJobsToBeSent() const;
    size_t numberOfJobsSent() const;
    static size_t numberOfAvailableSendJobs();

    /**
     * This function needs to be called once, before using any instance
     * of QueuedTransportLayer. It provides the pool of
     * ::transport::TransportMessageSendJob classes that all instances
     * of QueuedTransportLayer share.
     */
    static void initializeJobPool(SendJobPool& sendJobPool);
    static void initializeJobPool();

private:
    using TransportMessageSendJobList = ::estd::forward_list<TransportMessageSendJob>;

    static SendJobPool* sfpSendJobPool;

    static TransportMessageSendJob* getSendJob(
        TransportMessage& transportMessage,
        ITransportMessageProcessedListener* pNotificationListener);

    static void releaseSendJob(TransportMessageSendJob const& job);

    AbstractTransportLayer& fTransportLayer;
    TransportMessageSendJobList fJobsToBeSent;
    TransportMessageSendJobList fJobsSent;
};

/*
 *
 * Implementation
 *
 */

// virtual
inline AbstractTransportLayer::ErrorCode QueuedTransportLayer::init()
{
    return fTransportLayer.init();
}

// virtual
inline bool QueuedTransportLayer::shutdown(ShutdownDelegate const delegate)
{
    return fTransportLayer.shutdown(delegate);
}

inline size_t QueuedTransportLayer::numberOfJobsToBeSent() const { return fJobsToBeSent.size(); }

inline size_t QueuedTransportLayer::numberOfJobsSent() const { return fJobsSent.size(); }

// static
inline size_t QueuedTransportLayer::numberOfAvailableSendJobs()
{
    if (sfpSendJobPool != nullptr)
    {
        return sfpSendJobPool->size();
    }

    return 0U;
}

} // namespace transport

#endif /* GUARD_FEFB28C8_E743_4B4D_B771_AA61385C992E */
