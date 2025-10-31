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
    explicit IDiagDispatcher(IDiagSessionManager& sessionManager, DiagJobRoot& jobRoot)
    : fDiagJobRoot(jobRoot), fSessionManager(sessionManager), fEnabled(true)
    {}

    virtual uint16_t getSourceId() const = 0;

#ifdef IS_VARIANT_HANDLING_NEEDED
    virtual void setSourceId(uint16_t) = 0;
#endif

    virtual ::transport::AbstractTransportLayer::ErrorCode resume(
        ::transport::TransportMessage& transportMessage,
        ::transport::ITransportMessageProcessedListener* pNotificationListener)
        = 0;

protected:
    DiagJobRoot& fDiagJobRoot;

public:
    IDiagSessionManager& fSessionManager;
    bool fEnabled;
};

} // namespace uds
