// Copyright 2024 Accenture.

#pragma once

#include "uds/DiagReturnCode.h"
#include "uds/async/IAsyncDiagHelper.h"

#include <async/util/Call.h>

namespace uds
{
class AbstractDiagJob;
class IncomingDiagConnection;

/**
 * Helper class for handling nested diagnostic requests
 */
class AsyncDiagJobHelper
{
public:
    /**
     * Constructor
     * \param helper Reference to async helper class
     * \param job Reference to diag job
     * \param diagcontext Context to execute on
     */
    AsyncDiagJobHelper(
        IAsyncDiagHelper& asyncHelper, AbstractDiagJob& job, ::async::ContextType diagcontext);

    AsyncDiagJobHelper(AsyncDiagJobHelper const&)            = delete;
    AsyncDiagJobHelper& operator=(AsyncDiagJobHelper const&) = delete;

    /**
     * Check whether an asynchronous request is currently active
     */
    bool hasPendingAsyncRequest() const;

    /**
     * Get connection bound to the pending async request.
     * Note: Call this function only if hasPendingAsyncRequest() returns true
     * \return Reference to connection bound to this request
     */
    IncomingDiagConnection& getPendingConnection() const;

    /**
     * Try to enqueue a request.
     * \param connection Connection to enqueue
     * \return Result of enqueuing
     */
    DiagReturnCode::Type enqueueRequest(
        IncomingDiagConnection& connection, uint8_t const* request, uint16_t requestLength);

    /**
     * Start processing an async request.
     * \param connection Reference to connection bound to this request
     */
    void startAsyncRequest(IncomingDiagConnection& connection);

    /**
     * End processing an async request. This call will retrigger processing of
     * queued requests.
     */
    void endAsyncRequest();

private:
    void triggerNextRequests();

    IAsyncDiagHelper& fAsyncHelper;
    AbstractDiagJob& fJob;
    IncomingDiagConnection* fPendingAsyncConnection;
    ::estd::forward_list<IAsyncDiagHelper::StoredRequest> fPendingRequests;
    ::async::Function fTriggerNextRequests;
    ::async::ContextType fContext;
};

} // namespace uds

