// Copyright 2024 Accenture.

#pragma once

#include "uds/DiagnosisConfiguration.h"
#include "uds/IDiagDispatcher.h"
#include "uds/UdsConfig.h"

#include <async/Async.h>
#include <async/util/Call.h>
#include <etl/delegate.h>
#include <etl/uncopyable.h>
#include <transport/AbstractTransportLayer.h>
#include <transport/ITransportMessageProcessedListener.h>
#include <transport/ITransportMessageProvidingListener.h>
#include <transport/TransportJob.h>
#include <transport/TransportMessage.h>

#include <etl/queue.h>

namespace http
{
namespace html
{
class UdsController;
}
} // namespace http

namespace transport
{
class TransportJob;
}

namespace uds
{
class IDiagSessionManager;
class IncomingDiagConnection;

/**
 * DiagDispatcher is the ITransportMessageSender for a uds instance.
 *
 * \see     transport::AbstractTransportLayer
 */
class DiagDispatcher
: public IDiagDispatcher
, public transport::AbstractTransportLayer
, public transport::ITransportMessageProcessedListener
, public ::etl::uncopyable
{
public:
    /**
     * Constructor
     * \param   configuration   DiagnosisConfiguration holding
     * the configuration for this DiagDispatcher
     * \param   sessionManager  IDiagSessionManager
     * \param   context  Context used to handle DiagDispatcher's
     * timeouts
     */
    DiagDispatcher(
        ::etl::ipool& incomingDiagConnectionPool,
        ::etl::iqueue<transport::TransportJob>& sendJobQueue,
        DiagnosisConfiguration& configuration,
        IDiagSessionManager& sessionManager,
        DiagJobRoot& jobRoot);

    /**
     * \see     AbstractTransportLayer::init()
     * \post    isEnabled()
     */
    AbstractTransportLayer::ErrorCode init() override;

    /**
     * \see AbstractTransportLayer::shutdown()
     */
    bool shutdown(ShutdownDelegate delegate) override;

    /**
     * \see AbstractTransportLayer::send()
     */
    transport::AbstractTransportLayer::ErrorCode send(
        transport::TransportMessage& transportMessage,
        transport::ITransportMessageProcessedListener* pNotificationListener) override;

    /**
     * \see transport::ITransportMessageProcessedListener::transportMessageProcessed()
     */
    void transportMessageProcessed(
        transport::TransportMessage& transportMessage, ProcessingResult result) override;

    transport::AbstractTransportLayer::ErrorCode resume(
        transport::TransportMessage& transportMessage,
        transport::ITransportMessageProcessedListener* pNotificationListener) override;

    void processQueue();

    void shutdownIncomingConnections(::etl::delegate<void()> delegate);

private:
    using SendBusyResponseCallback
        = ::etl::delegate<void(transport::TransportMessage const* const)>;

    static uint8_t const BUSY_MESSAGE_LENGTH = 3U;

    friend class ::http::html::UdsController;
    friend class IncomingDiagConnection;

    void connectionManagerShutdownComplete();

    static void dispatchIncomingRequest(
        transport::TransportJob& job,
        DiagnosisConfiguration& configuration,
        DiagDispatcher& dispatcher,
        DiagJobRoot& diagJobRoot,
        transport::ITransportMessageProvidingListener& providingListener,
        transport::ITransportMessageProcessedListener* dispatcherProcessedListener,
        SendBusyResponseCallback sendBusyResponse);

    void sendBusyResponse(transport::TransportMessage const* const message);

    static bool isNegativeResponse(transport::TransportMessage const& transportMessage);

    static bool isFromValidSender(transport::TransportMessage const& transportMessage);

    /**
     * Functional requests might be routed to other busses if this UDS layer
     * is instantiated on a gateway. Thus, its content must not be altered
     * which is why a copy is made for further processing.
     */
    static transport::TransportMessage* copyFunctionalRequest(
        transport::TransportMessage& request,
        transport::ITransportMessageProvidingListener& providingListener,
        DiagnosisConfiguration& configuration);

    void diagConnectionTerminated(IncomingDiagConnection& diagConnection);

    void checkConnectionShutdownProgress();

    ::etl::ipool& incomingDiagConnectionPool;
    ::etl::iqueue<transport::TransportJob>& sendJobQueue;
    DiagnosisConfiguration& fConfiguration;
    ::etl::delegate<void()> fConnectionShutdownDelegate;
    bool fConnectionShutdownRequested;

    ShutdownDelegate fShutdownDelegate;
    ::transport::DefaultTransportMessageProcessedListener fDefaultTransportMessageProcessedListener;
    transport::TransportMessage fBusyMessage;
    uint8_t fBusyMessageBuffer[BUSY_MESSAGE_LENGTH + UdsVmsConstants::BUSY_MESSAGE_EXTRA_BYTES];
    ::async::Function fAsyncProcessQueue;
    DiagJobRoot& fDiagJobRoot;
};

} // namespace uds
