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
#include <transport/TransportMessage.h>

#include <etl/queue.h>

namespace http
{
namespace html
{
class UdsController;
}
} // namespace http

namespace uds
{
class IDiagSessionManager;
class IncomingDiagConnection;

struct TransportJob
{
    ::transport::TransportMessage* transportMessage                    = nullptr;
    ::transport::ITransportMessageProcessedListener* processedListener = nullptr;
};

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
        ::etl::iqueue<TransportJob>& sendJobQueue,
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
    ::etl::ipool& _incomingDiagConnectionPool;
    bool _connectionShutdownRequested = false;

    static uint8_t const BUSY_MESSAGE_LENGTH = 3U;

    friend class ::http::html::UdsController;
    friend class IncomingDiagConnection;

    void connectionManagerShutdownComplete();

    /**
     * Functional requests might be routed to other busses if this UDS layer
     * is instantiated on a gateway. Thus, its content must not be altered
     * which is why a copy is made for further processing.
     */
    void diagConnectionTerminated(IncomingDiagConnection& diagConnection);

    void checkConnectionShutdownProgress();

    ::etl::iqueue<TransportJob>& _sendJobQueue;
    DiagnosisConfiguration& _configuration;
    ::etl::delegate<void()> _connectionShutdownDelegate;
    ShutdownDelegate _shutdownDelegate;
    ::transport::DefaultTransportMessageProcessedListener _defaultTransportMessageProcessedListener;
    transport::TransportMessage _busyMessage;
    uint8_t _busyMessageBuffer[BUSY_MESSAGE_LENGTH + UdsVmsConstants::BUSY_MESSAGE_EXTRA_BYTES];
    ::async::Function _asyncProcessQueue;
    DiagJobRoot& _diagJobRoot;
};

// FIXME: This should not be public api, it is just exposed here because
// of strange usage in tests.
inline IncomingDiagConnection*
acquireIncomingDiagConnection(::etl::ipool& pool, ::async::ContextType context)
{
    if (pool.full())
    {
        return nullptr;
    }
    return pool.template create<IncomingDiagConnection>(context);
}

} // namespace uds
