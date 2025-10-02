// Copyright 2024 Accenture.

#pragma once

#include "platform/estdint.h"
#include "uds/connection/IncomingDiagConnection.h"

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
class DiagDispatcher;

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
        DiagDispatcher& diagDispatcher);

    IncomingDiagConnection* requestIncomingConnection(transport::TransportMessage& requestMessage);

    void diagConnectionTerminated(IncomingDiagConnection& diagConnection);

    bool isPendingActivated() const;

    uint16_t getSourceDiagId() const;

    uint8_t getBusId() const;

    void triggerResponseProcessing();

    void init() { fShutdownRequested = false; }

    void shutdown(::etl::delegate<void()> delegate);

private:
    friend class DiagDispatcher;

    void checkShutdownProgress();

    ::etl::delegate<void()> fShutdownDelegate;
    AbstractDiagnosisConfiguration& fConfiguration;
    transport::AbstractTransportLayer& fOutgoingTransportMessageSender;
    transport::ITransportMessageProvider& fOutgoingTransportMessageProvider;
    ::async::ContextType fContext;
    DiagDispatcher& fDiagDispatcher;
    bool fShutdownRequested;
};

} // namespace uds
