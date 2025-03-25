// Copyright 2024 Accenture.

#pragma once

#include "uds/UdsConfig.h"
#include "uds/base/AbstractDiagJob.h"
#include "uds/base/DiagJobRoot.h"
#include "uds/connection/IOutgoingDiagConnectionProvider.h"

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
class IDiagDispatcher : public IOutgoingDiagConnectionProvider
{
public:
    /**
     * Constructor
     * \param   sessionManager  IDiagSessionManager
     */
    explicit IDiagDispatcher(IDiagSessionManager& sessionManager, DiagJobRoot& jobRoot)
    : fDiagJobRoot(jobRoot), fSessionManager(sessionManager), fEnabled(true)
    {}

    IDiagSessionManager& getDiagSessionManager() const { return fSessionManager; }

    AbstractDiagJob::ErrorCode addAbstractDiagJob(AbstractDiagJob& job)
    {
        return fDiagJobRoot.addAbstractDiagJob(job);
    }

    void removeAbstractDiagJob(AbstractDiagJob& job) { fDiagJobRoot.removeAbstractDiagJob(job); }

    bool isEnabled() const { return fEnabled; }

    void disable() { fEnabled = false; }

    void enable() { fEnabled = true; }

    virtual uint16_t getSourceId() const = 0;

#ifdef IS_VARIANT_HANDLING_NEEDED
    virtual void setSourceId(uint16_t) = 0;
#endif

    /*
     * \param tmsg - preset message to trigger uds flow
     * \return - 0 successful, 1 - not successful
     * \note - will only be executed in case no incoming diag message
     */
    virtual uint8_t dispatchTriggerEventRequest(transport::TransportMessage& tmsg) = 0;

protected:
    DiagJobRoot& fDiagJobRoot;
    IDiagSessionManager& fSessionManager;

    bool fEnabled;
};

} // namespace uds

