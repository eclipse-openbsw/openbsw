// Copyright 2024 Accenture.

#pragma once

#include "platform/estdint.h"
#include "uds/connection/IOutgoingDiagConnectionProvider.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/connection/ManagedOutgoingDiagConnection.h"

#include <etl/delegate.h>
#include <etl/intrusive_list.h>
#include <etl/uncopyable.h>

namespace transport
{
class TransportMessage;
class AbstractTransportLayer;
class ITransportMessageProvider;
} // namespace transport

namespace uds
{
class AbstractDiagnosisConfiguration;
class OutgoingDiagConnection;
class DiagDispatcher2;

class DiagConnectionManager : public etl::uncopyable
{
public:
    static uint32_t const INCOMING_REQUEST_TIMEOUT    = 60000U;
    static uint32_t const PENDING_INTERVAL            = 4000U;
    static uint32_t const PENDING_OFFSET              = 300U;
    static uint16_t const MONITOR_CONNECTION_INTERVAL = 20U;

    DiagConnectionManager(
        AbstractDiagnosisConfiguration& configuration,
        transport::AbstractTransportLayer& outgoingSender,
        transport::ITransportMessageProvider& outgoingProvider,
        ::async::ContextType context,
        DiagDispatcher2& diagDispatcher);

    IOutgoingDiagConnectionProvider::ErrorCode requestOutgoingConnection(
        uint16_t targetId,
        OutgoingDiagConnection*& pOutgoingConnection,
        transport::TransportMessage* pRequestMessage);

    IncomingDiagConnection* requestIncomingConnection(transport::TransportMessage& requestMessage);

    void diagConnectionTerminated(IncomingDiagConnection& diagConnection);

    void diagConnectionTerminated(ManagedOutgoingDiagConnection& diagConnection);

    bool isPendingActivated() const;

    ManagedOutgoingDiagConnection*
    getExpectingConnection(transport::TransportMessage const& transportMessage);

    uint16_t getSourceDiagId() const;

    uint8_t getBusId() const;

    /**
     * Checks if any of the existing OutgoingDiagConnections are conflicting
     * \param connection  OutgoingDiagConnection to check
     * \return *          - true: There is a conflicting connection
     *          - false: No conflicts were found
     */
    bool hasConflictingConnection(OutgoingDiagConnection const& connection);

    void triggerResponseProcessing();

    void processPendingResponses();

    void init() { fShutdownRequested = false; }

    void shutdown(::etl::delegate<void()> delegate);

private:
    friend class DiagDispatcher2;

    using ManagedOutgoingDiagConnectionList
        = ::etl::intrusive_list<ManagedOutgoingDiagConnection, etl::bidirectional_link<0>>;

    void checkShutdownProgress();

    ManagedOutgoingDiagConnectionList& getReleasedConnections()
    {
        return fReleasedOutgoingDiagConnections;
    }

    ::etl::delegate<void()> fShutdownDelegate;
    AbstractDiagnosisConfiguration& fConfiguration;
    transport::AbstractTransportLayer& fOutgoingTransportMessageSender;
    transport::ITransportMessageProvider& fOutgoingTransportMessageProvider;
    ::async::ContextType fContext;
    DiagDispatcher2& fDiagDispatcher;
    ::etl::intrusive_list<ManagedOutgoingDiagConnection, ::etl::bidirectional_link<0>>&
        fOutgoingDiagConnections;
    ::etl::intrusive_list<ManagedOutgoingDiagConnection, ::etl::bidirectional_link<0>>
        fReleasedOutgoingDiagConnections;
    bool fShutdownRequested;
};

} // namespace uds
