/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "uds/DiagnosisConfiguration.h"
#include "uds/IDiagDispatcher.h"
#include "uds/UdsConfig.h"

#if UDS_ENABLE_OUTGOING
#include "uds/DiagReturnCode.h"
#include "uds/connection/IOutgoingDiagConnectionProvider.h"
#include "uds/connection/OutgoingDiagConnectionManager.h"

#include <etl/span.h>
#endif

#include <async/Async.h>
#include <async/util/Call.h>
#include <etl/delegate.h>
#include <etl/queue.h>
#include <etl/uncopyable.h>
#include <transport/AbstractTransportLayer.h>
#include <transport/ITransportMessageProcessedListener.h>
#include <transport/ITransportMessageProvidingListener.h>
#include <transport/TransportMessage.h>

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
#if UDS_ENABLE_OUTGOING
class ManagedOutgoingDiagConnection;
#endif

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
#if UDS_ENABLE_OUTGOING
, public IOutgoingDiagConnectionProvider
#endif
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

#if UDS_ENABLE_OUTGOING
    template<size_t QUEUE_SIZE>
    DiagDispatcher(
        ::etl::ipool& incomingDiagConnectionPool,
        ::etl::iqueue<TransportJob>& sendJobQueue,
        DiagnosisConfiguration& configuration,
        IDiagSessionManager& sessionManager,
        DiagJobRoot& jobRoot,
        ::etl::span<ManagedOutgoingDiagConnection> outgoingConnections,
        ::etl::span<::etl::queue<TransportJob, QUEUE_SIZE>> responseQueues)
    : IDiagDispatcher(sessionManager)
    , AbstractTransportLayer(configuration.DiagBusId)
    , _incomingDiagConnectionPool(incomingDiagConnectionPool)
    , _sendJobQueue(sendJobQueue)
    , _configuration(configuration)
    , _busyMessageBuffer()
    , _asyncProcessQueue(
          ::async::Function::CallType::create<DiagDispatcher, &DiagDispatcher::processQueue>(*this))
    , _diagJobRoot(jobRoot)
    , _outgoingConnectionManager(
          configuration,
          *this,
          fProvidingListenerHelper,
          *this,
          outgoingConnections,
          responseQueues)
    {
        _busyMessage.init(
            &_busyMessageBuffer[0],
            BUSY_MESSAGE_LENGTH + UdsVmsConstants::BUSY_MESSAGE_EXTRA_BYTES);
        _busyMessage.resetValidBytes();
        (void)_busyMessage.append(DiagReturnCode::NEGATIVE_RESPONSE_IDENTIFIER);
        (void)_busyMessage.append(0x00U);
        (void)_busyMessage.append(static_cast<uint8_t>(DiagReturnCode::ISO_BUSY_REPEAT_REQUEST));
        _busyMessage.setPayloadLength(BUSY_MESSAGE_LENGTH);
    }
#endif

    /**
     * \see     AbstractTransportLayer::init()
     * \post    isEnabled()
     */
    AbstractTransportLayer::ErrorCode init() override;

    /**
     * \see AbstractTransportLayer::shutdown()
     */
    bool shutdown(ShutdownDelegate delegate) override;

#if UDS_ENABLE_OUTGOING
    /**
     * \see IOutgoingDiagConnectionProvider::getOutgoingDiagConnection()
     */
    IOutgoingDiagConnectionProvider::ErrorCode getOutgoingDiagConnection(
        uint16_t targetId,
        OutgoingDiagConnection*& pConnection,
        transport::TransportMessage* pRequestMessage) override;
#endif

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
#if UDS_ENABLE_OUTGOING
    friend class OutgoingDiagConnectionManager;
#endif

    void connectionManagerShutdownComplete();

    /**
     * Functional requests might be routed to other busses if this UDS layer
     * is instantiated on a gateway. Thus, its content must not be altered
     * which is why a copy is made for further processing.
     */
    void diagConnectionTerminated(IncomingDiagConnection& diagConnection);

    void checkConnectionShutdownProgress();

    void trigger();

    ::etl::iqueue<TransportJob>& _sendJobQueue;
    DiagnosisConfiguration& _configuration;
    ::etl::delegate<void()> _connectionShutdownDelegate;
    ShutdownDelegate _shutdownDelegate;
    ::transport::DefaultTransportMessageProcessedListener _defaultTransportMessageProcessedListener;
    transport::TransportMessage _busyMessage;
    uint8_t _busyMessageBuffer[BUSY_MESSAGE_LENGTH + UdsVmsConstants::BUSY_MESSAGE_EXTRA_BYTES];
    ::async::Function _asyncProcessQueue;
    DiagJobRoot& _diagJobRoot;
#if UDS_ENABLE_OUTGOING
    OutgoingDiagConnectionManager _outgoingConnectionManager;
#endif
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
