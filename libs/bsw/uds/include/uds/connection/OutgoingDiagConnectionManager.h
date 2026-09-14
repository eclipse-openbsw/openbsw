/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <etl/error_handler.h>
#include <etl/intrusive_list.h>
#include <etl/queue.h>
#include <etl/span.h>
#include <etl/uncopyable.h>

#include <cstdint>

#include "uds/connection/IOutgoingDiagConnectionProvider.h"
#include "uds/connection/ManagedOutgoingDiagConnection.h"

namespace transport
{
class AbstractTransportLayer;
class TransportMessage;
class ITransportMessageProvider;
} // namespace transport

namespace uds
{
class DiagnosisConfiguration;
class OutgoingDiagConnection;
class DiagDispatcher;
struct TransportJob;

class OutgoingDiagConnectionManager : public etl::uncopyable
{
public:
    OutgoingDiagConnectionManager(
        DiagnosisConfiguration& configuration,
        transport::AbstractTransportLayer& outgoingSender,
        transport::ITransportMessageProvider& outgoingProvider,
        DiagDispatcher& diagDispatcher);

    template<size_t QUEUE_SIZE>
    OutgoingDiagConnectionManager(
        DiagnosisConfiguration& configuration,
        transport::AbstractTransportLayer& outgoingSender,
        transport::ITransportMessageProvider& outgoingProvider,
        DiagDispatcher& diagDispatcher,
        ::etl::span<ManagedOutgoingDiagConnection> outgoingConnections,
        ::etl::span<::etl::queue<TransportJob, QUEUE_SIZE>> responseQueues)
    : _configuration(configuration)
    , _outgoingTransportMessageSender(outgoingSender)
    , _outgoingTransportMessageProvider(outgoingProvider)
    , _diagDispatcher(diagDispatcher)
    , _outgoingDiagConnections()
    , _releasedOutgoingDiagConnections()
    , _shutdownRequested(false)
    {
        ETL_ASSERT(
            outgoingConnections.size() == responseQueues.size(),
            ETL_ERROR_GENERIC("Not enough response queues for outgoing connections"));

        for (size_t i = 0U; i < outgoingConnections.size(); ++i)
        {
            outgoingConnections[i].setResponseQueue(responseQueues[i]);
            _outgoingDiagConnections.push_back(outgoingConnections[i]);
        }
    }

    IOutgoingDiagConnectionProvider::ErrorCode requestOutgoingConnection(
        uint16_t targetAddress,
        OutgoingDiagConnection*& pOutgoingConnection,
        transport::TransportMessage* pRequestMessage);

    void diagConnectionTerminated(ManagedOutgoingDiagConnection& diagConnection);

    ManagedOutgoingDiagConnection*
    getExpectingConnection(transport::TransportMessage const& transportMessage);

    /**
     * Checks if any of the existing OutgoingDiagConnections are conflicting
     */
    bool hasConflictingConnection(OutgoingDiagConnection const& connection);

    void triggerResponseProcessing();

    void processPendingResponses();

    void init() { _shutdownRequested = false; }

    void shutdown();

private:
    friend class DiagDispatcher;

    using ManagedOutgoingDiagConnectionList
        = ::etl::intrusive_list<ManagedOutgoingDiagConnection, etl::bidirectional_link<0>>;

    void checkShutdownProgress();

    ManagedOutgoingDiagConnectionList& getReleasedConnections()
    {
        return _releasedOutgoingDiagConnections;
    }

    DiagnosisConfiguration& _configuration;
    transport::AbstractTransportLayer& _outgoingTransportMessageSender;
    transport::ITransportMessageProvider& _outgoingTransportMessageProvider;
    DiagDispatcher& _diagDispatcher;
    ManagedOutgoingDiagConnectionList _outgoingDiagConnections;
    ManagedOutgoingDiagConnectionList _releasedOutgoingDiagConnections;
    bool _shutdownRequested;
};

} // namespace uds
