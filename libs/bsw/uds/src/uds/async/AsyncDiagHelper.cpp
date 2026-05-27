/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/async/AsyncDiagHelper.h"

#include "uds/connection/IncomingDiagConnection.h"

#include <transport/TransportMessage.h>

namespace uds
{
AsyncDiagHelper::AsyncDiagHelper(
    StoredRequestPool& storedRequestPool, ::async::ContextType diagContext)
: IAsyncDiagHelper()
, AbstractDiagJob(nullptr, 0U, 0U)
, fStoredRequestPool(storedRequestPool)
, fDiagContext(diagContext)
{}

DiagReturnCode::Type
AsyncDiagHelper::verify(uint8_t const* const /* request */, uint16_t const /* requestLength */)
{
    return DiagReturnCode::OK;
}

::async::ContextType AsyncDiagHelper::getDiagContext() const { return fDiagContext; }

AsyncDiagHelper::StoredRequest* AsyncDiagHelper::allocateRequest(
    IncomingDiagConnection& connection, uint8_t const* request, uint16_t requestLength)
{
    if (!fStoredRequestPool.full())
    {
        return fStoredRequestPool.template create<
            AsyncDiagHelper::StoredRequest,
            IncomingDiagConnection&,
            uint8_t const*,
            uint16_t>(connection, etl::move(request), etl::move(requestLength));
    }

    return nullptr;
}

void AsyncDiagHelper::processAndReleaseRequest(AbstractDiagJob& job, StoredRequest& request)
{
    IncomingDiagConnection& connection = request.getConnection();
    DiagReturnCode::Type const responseCode
        = job.process(connection, request.getRequest(), request.getRequestLength());
    fStoredRequestPool.destroy(&request);
    if (responseCode != DiagReturnCode::OK)
    {
        (void)connection.sendNegativeResponse(static_cast<uint8_t>(responseCode), *this);
        connection.terminate();
    }
}

} // namespace uds
