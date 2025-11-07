// Copyright 2024 Accenture.

#pragma once

#include "transport/AbstractTransportLayer.h"
#include "uds/UdsConfig.h"
#include "uds/base/AbstractDiagJob.h"
#include "uds/base/DiagJobRoot.h"

namespace uds
{
class IDiagSessionManager;

/**
 * IDiagDispatcher is an abstract class used by
 * services which need access to the diag dispatcher
 *
 *
 *
 *
 * \see     IOutgoingDiagConnectionProvider
 */
class IDiagDispatcher
{
public:
    /**
     * Constructor
     * \param   sessionManager  IDiagSessionManager
     */
    explicit IDiagDispatcher(IDiagSessionManager& sessionManager)
    : fSessionManager(sessionManager), fEnabled(true)
    {}

    virtual uint16_t getDispatcherSourceId() const = 0;

    virtual ::transport::AbstractTransportLayer::ErrorCode resume(
        ::transport::TransportMessage& transportMessage,
        ::transport::ITransportMessageProcessedListener* pNotificationListener)
        = 0;

public:
    IDiagSessionManager& fSessionManager;
    bool fEnabled;
};

} // namespace uds
