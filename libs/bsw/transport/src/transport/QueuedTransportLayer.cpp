// Copyright 2024 Accenture.

#include "transport/QueuedTransportLayer.h"

#include "common/busid/BusId.h"
#include "transport/TransportLogger.h"
#include "transport/TransportMessage.h"

#include <async/Async.h>

#include <platform/config.h>

namespace transport
{
using ::util::logger::Logger;
using ::util::logger::TRANSPORT;

// static
QueuedTransportLayer::SendJobPool* QueuedTransportLayer::sfpSendJobPool = nullptr;

// static
void QueuedTransportLayer::initializeJobPool(SendJobPool& sendJobPool)
{
    sfpSendJobPool = &sendJobPool;
}

// static
void QueuedTransportLayer::initializeJobPool() { sfpSendJobPool = nullptr; }

// explicit
QueuedTransportLayer::QueuedTransportLayer(AbstractTransportLayer& transportLayer)
: AbstractTransportLayer(transportLayer.getBusId())
, fTransportLayer(transportLayer)
, fJobsToBeSent()
, fJobsSent()
{
    transportLayer.setTransportMessageListener(&getProvidingListenerHelper());
    transportLayer.setTransportMessageProvider(&getProvidingListenerHelper());
}

AbstractTransportLayer::ErrorCode QueuedTransportLayer::send(
    TransportMessage& transportMessage,
    ITransportMessageProcessedListener* const pNotificationListener)
{
    AbstractTransportLayer::ErrorCode status = ErrorCode::TP_SEND_FAIL;

    ::async::ModifiableLockType mutex;

    if (fJobsToBeSent.empty())
    { // no jobs in queue --> try to send
        TransportMessageSendJob* const pJob = getSendJob(transportMessage, pNotificationListener);
        if (pJob == nullptr)
        {
            Logger::error(
                TRANSPORT,
                "QueuedTransportLayer(%s): no SendJob left!",
                ::common::busid::BusIdTraits::getName(getBusId()));
        }
        else
        {
            fJobsSent.push_back(*pJob);

            mutex.unlock();
            status = fTransportLayer.send(transportMessage, this);
            mutex.lock();

            if (status == ErrorCode::TP_QUEUE_FULL)
            {
                // job has NOT been sent --> remove from pending send jobs
                fJobsSent.remove(*pJob);
                fJobsToBeSent.push_back(*pJob);
                status = ErrorCode::TP_OK;
            }
            else if (status != ErrorCode::TP_OK)
            {
                Logger::warn(
                    TRANSPORT,
                    "QueuedTransportLayer(%s): immediate send 0x%x --> 0x%x failed with status %d",
                    ::common::busid::BusIdTraits::getName(getBusId()),
                    transportMessage.getSourceId(),
                    transportMessage.getTargetId(),
                    status);
                // job has NOT been sent --> remove from pending send jobs
                fJobsSent.remove(*pJob);
                releaseSendJob(*pJob);
            }
            else
            {
                // do nothing
            }
        }
    }
    else
    { // at least one job in queue --> enqueue
        Logger::debug(
            TRANSPORT,
            "QueuedTransportLayer(%s): enqueuing message 0x%x --> 0x%x!",
            ::common::busid::BusIdTraits::getName(getBusId()),
            transportMessage.getSourceId(),
            transportMessage.getTargetId());
        TransportMessageSendJob* const pJob = getSendJob(transportMessage, pNotificationListener);
        if (pJob != nullptr)
        {
            fJobsToBeSent.push_back(*pJob);
            status = ErrorCode::TP_OK;
        }
        else
        {
            Logger::error(
                TRANSPORT,
                "QueuedTransportLayer(%s): no SendJob left!",
                ::common::busid::BusIdTraits::getName(getBusId()));
        }
    }
    return status;
}

void QueuedTransportLayer::transportMessageProcessed(
    TransportMessage& transportMessage,
    ITransportMessageProcessedListener::ProcessingResult const result)
{
    bool isMessageInSentList = false;
    ::async::ModifiableLockType mutex;

    TransportMessageSendJobList::iterator const end = fJobsSent.end();
    for (TransportMessageSendJobList::iterator itr = fJobsSent.begin(); itr != end; ++itr)
    {
        TransportMessageSendJob& job = *itr;
        if (job.getTransportMessage() == &transportMessage)
        {
            ITransportMessageProcessedListener* const pListener
                = job.getTransportMessageProcessedListener();
            if (pListener != nullptr)
            {
                mutex.unlock();
                pListener->transportMessageProcessed(transportMessage, result);
                mutex.lock();
            }
            isMessageInSentList = true;
            fJobsSent.remove(job);
            releaseSendJob(job);
            break;
        }
    }
    if (!isMessageInSentList)
    {
        Logger::error(
            TRANSPORT,
            "QueuedTransportLayer::transportMessageProcessed(): unknown transportMessage (0x%x --> "
            "0x%x)",
            transportMessage.getSourceId(),
            transportMessage.getTargetId());
    }
    AbstractTransportLayer::ErrorCode status = ErrorCode::TP_OK;

    while ((!fJobsToBeSent.empty()) && (status == ErrorCode::TP_OK))
    {
        TransportMessageSendJob& job = fJobsToBeSent.front();
        fJobsToBeSent.pop_front();
        fJobsSent.push_back(job);
        TransportMessage& tpMessage = *job.getTransportMessage();

        mutex.unlock();
        status = fTransportLayer.send(tpMessage, this);
        mutex.lock();

        if (status != ErrorCode::TP_OK)
        { // job has NOT been sent --> remove from pending send jobs
            fJobsSent.remove(job);
            if (status == ErrorCode::TP_QUEUE_FULL)
            {
                fJobsToBeSent.push_front(job);
            }
            else
            {
                Logger::error(
                    TRANSPORT,
                    "QueuedTransportLayer::transportMessageProcessed(): failed to send message "
                    "(0x%x --> 0x%x) with status %d",
                    tpMessage.getSourceId(),
                    tpMessage.getTargetId(),
                    status);
                ITransportMessageProcessedListener* const pListener
                    = job.getTransportMessageProcessedListener();
                if (pListener != nullptr)
                {
                    mutex.unlock();
                    pListener->transportMessageProcessed(
                        tpMessage,
                        ITransportMessageProcessedListener::ProcessingResult::
                            PROCESSED_ERROR_GENERAL);
                    mutex.lock();
                }
                releaseSendJob(job);
            }
        }
    }
}

// static
TransportMessageSendJob* QueuedTransportLayer::getSendJob(
    TransportMessage& transportMessage,
    ITransportMessageProcessedListener* const pNotificationListener)
{
    if (sfpSendJobPool != nullptr)
    {
        if (!sfpSendJobPool->empty())
        {
            TransportMessageSendJob& job
                = sfpSendJobPool->allocate().construct(&transportMessage, pNotificationListener);
            return &job;
        }
    }
    return nullptr;
}

// static
void QueuedTransportLayer::releaseSendJob(TransportMessageSendJob const& job)
{
    // This cannot be reached if sfpSendJobPool is not set
    sfpSendJobPool->release(job);
}

} // namespace transport
