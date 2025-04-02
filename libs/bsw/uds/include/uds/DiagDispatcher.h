// Copyright 2024 Accenture.

#pragma once

#include "uds/DiagnosisConfiguration.h"
#include "uds/IDiagDispatcher.h"
#include "uds/UdsConfig.h"
#include "uds/connection/DiagConnectionManager.h"
#include "uds/resume/IResumableDiagDispatcher.h"

#include <async/Async.h>
#include <async/util/Call.h>
#include <transport/AbstractTransportLayer.h>
#include <transport/ITransportMessageProcessedListener.h>
#include <transport/TransportMessage.h>

#include <estd/uncopyable.h>

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

/**
 * DiagDispatcher is the ITransportMessageSender for a uds instance.
 *
 * \see     transport::AbstractTransportLayer
 */
class DiagDispatcher2
: public IResumableDiagDispatcher
, public transport::AbstractTransportLayer
, public transport::ITransportMessageProcessedListener
{
    UNCOPYABLE(DiagDispatcher2);

public:
    /**
     * Constructor
     * \param   configuration   AbstractDiagnosisConfiguration holding
     * the configuration for this DiagDispatcher
     * \param   sessionManager  IDiagSessionManager
     * \param   context  Context used to handle DiagDispatcher2's
     * timeouts
     */
    DiagDispatcher2(
        AbstractDiagnosisConfiguration& configuration,
        IDiagSessionManager& sessionManager,
        DiagJobRoot& jobRoot,
        ::async::ContextType context);

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
     * \see IOutgoingDiagConnectionProvider::getOutgoingDiagConnection()
     */
    IOutgoingDiagConnectionProvider::ErrorCode getOutgoingDiagConnection(
        uint16_t targetId,
        OutgoingDiagConnection*& pConnection,
        transport::TransportMessage* pRequestMessage) override;

    /**
     * \see transport::ITransportMessageProcessedListener::transportMessageProcessed()
     */
    void transportMessageProcessed(
        transport::TransportMessage& transportMessage, ProcessingResult result) override;

    /**
     * \see uds::IResumableDiagDispatcher::resume()
     */
    transport::AbstractTransportLayer::ErrorCode resume(
        transport::TransportMessage& transportMessage,
        transport::ITransportMessageProcessedListener* pNotificationListener) override;

    void processQueue();

    DiagConnectionManager& getConnectionManager() { return fConnectionManager; }

    uint16_t getSourceId() const override { return fConfiguration.DiagAddress; }

#ifdef IS_VARIANT_HANDLING_NEEDED
    virtual void setSourceId(uint16_t diagAddress) { fConfiguration.DiagAddress = diagAddress; }
#endif

    /*
     * \param msg - preset message to trigger uds flow
     * \return - 0 successful, 1 - not successful
     * \note - will only be executed in case no incoming diag message
     */
    uint8_t dispatchTriggerEventRequest(transport::TransportMessage& tmsg) override;

private:
    // workaround for large non virtual thunks
    bool shutdown_local(ShutdownDelegate delegate);
    transport::AbstractTransportLayer::ErrorCode send_local(
        transport::TransportMessage& transportMessage,
        transport::ITransportMessageProcessedListener* pNotificationListener);

    static uint8_t const BUSY_MESSAGE_LENGTH = 3U;

    friend class ::http::html::UdsController;
    friend class DiagConnectionManager;

    class DefaultTransportMessageProcessedListener
    : public transport::ITransportMessageProcessedListener
    {
        UNCOPYABLE(DefaultTransportMessageProcessedListener);

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

    void dispatchIncomingRequest(transport::TransportJob& job);

    void sendBusyResponse(transport::TransportMessage const* const message);

    static bool isNegativeResponse(transport::TransportMessage const& transportMessage);

    static bool isFromValidSender(transport::TransportMessage const& transportMessage);

    /**
     * Functional requests might be routed to other busses if this UDS layer
     * is instantiated on a gateway. Thus, its content must not be altered
     * which is why a copy is made for further processing.
     */
    transport::TransportMessage* copyFunctionalRequest(transport::TransportMessage& request);

    AbstractDiagnosisConfiguration& fConfiguration;
    DiagConnectionManager fConnectionManager;

    ShutdownDelegate fShutdownDelegate;
    DefaultTransportMessageProcessedListener fDefaultTransportMessageProcessedListener;
    transport::TransportMessage fBusyMessage;
    uint8_t fBusyMessageBuffer[BUSY_MESSAGE_LENGTH + UdsVmsConstants::BUSY_MESSAGE_EXTRA_BYTES];
    ::async::Function fAsyncProcessQueue;
};

} // namespace uds
