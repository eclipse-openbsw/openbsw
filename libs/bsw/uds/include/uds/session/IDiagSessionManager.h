// Copyright 2024 Accenture.

#pragma once

#include "uds/base/AbstractDiagJob.h"

namespace uds
{
class DiagSession;
class DiagSubSession;
class IDiagSessionChangedListener;
class IncomingDiagConnection;

/**
 * Interface for handling UDS sessions.
 */
class IDiagSessionManager
{
public:
#ifdef UNIT_TEST
    /**
     * Destructor
     */
    virtual ~IDiagSessionManager() {}
#endif

    /**
     * \return  current active DiagSession
     * \note    if applicable DiagSession provides DiagSubSession
     */
    virtual DiagSession const& getActiveSession() const = 0;

    /**
     * Call this method to renew the timeout before the current session is left.
     * This method must be called upon completion of a diagnosis request, i.e.
     * when the response is sent (neg. resp. pending must be ignored!)
     */
    virtual void startSessionTimeout() = 0;

    /**
     * Call this method upon reception of a diagnosis request to stay during its
     * execution in the current session.
     */
    virtual void stopSessionTimeout() = 0;

    /**
     * Current status of S3 timeout
     * \return status of S3 timeout
     * - true: there is currently no diagnosis job running
     * - false: either no S3 session is active or there's a long running job active
     */
    virtual bool isSessionTimeoutActive() = 0;

    /**
     * Requests change to default session
     * \return change successful
     * - true possible
     * - fale current session does not permit transistion
     */
    virtual void resetToDefaultSession() = 0;

    /**
     * Optionally requests session manager to persist the current diagnostic session and
     * restore it after device reset.
     * \return task accepted
     * - true diagnostic session will be restored
     * - false currently not possible
     */
    virtual bool persistAndRestoreSession() { return false; }

    virtual DiagReturnCode::Type acceptedJob(
        IncomingDiagConnection& connection,
        AbstractDiagJob const& job,
        uint8_t const request[],
        uint16_t requestLength)
        = 0;

    virtual void responseSent(
        IncomingDiagConnection& connection,
        DiagReturnCode::Type result,
        uint8_t const response[],
        uint16_t responseLength)
        = 0;

    virtual void addDiagSessionListener(IDiagSessionChangedListener&) = 0;

    virtual void removeDiagSessionListener(IDiagSessionChangedListener&) = 0;
};

} // namespace uds
