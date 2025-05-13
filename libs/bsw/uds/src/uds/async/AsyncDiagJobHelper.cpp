// Copyright 2024 Accenture.

#include "uds/async/AsyncDiagJobHelper.h"

#include <transport/TransportMessage.h>

namespace uds
{
AsyncDiagJobHelper::AsyncDiagJobHelper(IAsyncDiagHelper& asyncHelper, AbstractDiagJob& job)
: fAsyncHelper(asyncHelper)
, fJob(job)
, fPendingAsyncConnection(nullptr)
, fPendingRequests()
, fTriggerNextRequests(
      ::async::Function::CallType::
          create<AsyncDiagJobHelper, &AsyncDiagJobHelper::triggerNextRequests>(*this))
{}

bool AsyncDiagJobHelper::hasPendingAsyncRequest() const
{
    return fPendingAsyncConnection != nullptr;
}

IncomingDiagConnection& AsyncDiagJobHelper::getPendingConnection() const
{
    return *fPendingAsyncConnection;
}

DiagReturnCode::Type AsyncDiagJobHelper::enqueueRequest(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    IAsyncDiagHelper::StoredRequest* const storedRequest
        = fAsyncHelper.allocateRequest(connection, request, requestLength);
    if (storedRequest != nullptr)
    {
        fPendingRequests.push_back(*storedRequest);
        return DiagReturnCode::OK;
    }
    else
    {
        return DiagReturnCode::ISO_BUSY_REPEAT_REQUEST;
    }
}

void AsyncDiagJobHelper::startAsyncRequest(IncomingDiagConnection& connection)
{
    fPendingAsyncConnection = &connection;
}

void AsyncDiagJobHelper::endAsyncRequest()
{
    if (fPendingAsyncConnection != nullptr)
    {
        ::async::execute(fAsyncHelper.getDiagContext(), fTriggerNextRequests);
    }
}

void AsyncDiagJobHelper::triggerNextRequests()
{
    fPendingAsyncConnection = nullptr;
    while ((fPendingAsyncConnection == nullptr) && (!fPendingRequests.empty()))
    {
        IAsyncDiagHelper::StoredRequest& storedRequest = fPendingRequests.front();
        fPendingRequests.pop_front();
        fAsyncHelper.processAndReleaseRequest(fJob, storedRequest);
    }
}

} // namespace uds
