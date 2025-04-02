// Copyright 2024 Accenture.

#include "uds/async/AsyncDiagHelper.h"

#include "uds/connection/IncomingDiagConnection.h"

#include <transport/TransportMessage.h>

namespace uds
{
AsyncDiagHelper::AsyncDiagHelper(StoredRequestPool& storedRequestPool)
: IAsyncDiagHelper(), AbstractDiagJob(nullptr, 0U, 0U), fStoredRequestPool(storedRequestPool)
{}

DiagReturnCode::Type
AsyncDiagHelper::verify(uint8_t const* const /* request */, uint16_t const /* requestLength */)
{
    return DiagReturnCode::OK;
}

AsyncDiagHelper::StoredRequest* AsyncDiagHelper::allocateRequest(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    if (!fStoredRequestPool.empty())
    {
        return &fStoredRequestPool.allocate().construct(connection, request, requestLength);
    }

    return nullptr;
}

void AsyncDiagHelper::processAndReleaseRequest(AbstractDiagJob& job, StoredRequest& request)
{
    IncomingDiagConnection& connection = request.getConnection();
    DiagReturnCode::Type const responseCode
        = job.process(connection, request.getRequest(), request.getRequestLength());
    fStoredRequestPool.release(request);
    if (responseCode != DiagReturnCode::OK)
    {
        (void)connection.sendNegativeResponse(static_cast<uint8_t>(responseCode), *this);
        connection.terminate();
    }
}

} // namespace uds
