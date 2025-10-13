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

#ifdef IS_VARIANT_HANDLING_NEEDED
#include "uds/DiagnosisConfiguration.h"
#endif

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
     * \param   configuration   AbstractDiagnosisConfiguration holding
     * the configuration for this DiagDispatcher
     * \param   sessionManager  IDiagSessionManager
     * \param   context  Context used to handle DiagDispatcher's
     * timeouts
     */
    DiagDispatcher(
        AbstractDiagnosisConfiguration& configuration,
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

    uint16_t getDispatcherSourceId() const override { return fConfiguration.DiagAddress; }

#ifdef IS_VARIANT_HANDLING_NEEDED
    virtual void setSourceAddress(uint16_t diagAddress)
    {
        fConfiguration.DiagAddress = diagAddress;
    }
#endif

private:
    using SendBusyResponseCallback
        = ::etl::delegate<void(transport::TransportMessage const* const)>;

    // workaround for large non virtual thunks
    bool shutdown_local(ShutdownDelegate delegate);
    transport::AbstractTransportLayer::ErrorCode send_local(
        transport::TransportMessage& transportMessage,
        transport::ITransportMessageProcessedListener* pNotificationListener);

    static uint8_t const BUSY_MESSAGE_LENGTH = 3U;

    friend class ::http::html::UdsController;
    friend class IncomingDiagConnection;

    class DefaultTransportMessageProcessedListener
    : public transport::ITransportMessageProcessedListener
    , public ::etl::uncopyable
    {
    public:
        DefaultTransportMessageProcessedListener() {}

        /**
         * \see transport::ITransportMessageProcessedListener::transportMessageProcessed()
         */

        void transportMessageProcessed(
            transport::TransportMessage& /* transportMessage */,
            ProcessingResult const /* result */) override
        {}
    };

    void connectionManagerShutdownComplete();

    transport::AbstractTransportLayer::ErrorCode enqueueMessage(
        transport::TransportMessage& transportMessage,
        transport::ITransportMessageProcessedListener* pNotificationListener);

    void trigger();

    static void dispatchIncomingRequest(
        transport::TransportJob& job,
        AbstractDiagnosisConfiguration& configuration,
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
        AbstractDiagnosisConfiguration& configuration);

    IncomingDiagConnection* requestIncomingConnection(transport::TransportMessage& requestMessage);

    void diagConnectionTerminated(IncomingDiagConnection& diagConnection);

    void checkConnectionShutdownProgress();

    AbstractDiagnosisConfiguration& fConfiguration;
    ::etl::delegate<void()> fConnectionShutdownDelegate;
    bool fConnectionShutdownRequested;

    ShutdownDelegate fShutdownDelegate;
    DefaultTransportMessageProcessedListener fDefaultTransportMessageProcessedListener;
    transport::TransportMessage fBusyMessage;
    uint8_t fBusyMessageBuffer[BUSY_MESSAGE_LENGTH + UdsVmsConstants::BUSY_MESSAGE_EXTRA_BYTES];
    ::async::Function fAsyncProcessQueue;
    DiagJobRoot& fDiagJobRoot;
};

} // namespace uds
