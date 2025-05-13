// Copyright 2024 Accenture.

#pragma once

#include "uds/async/IAsyncDiagHelper.h"
#include "uds/base/AbstractDiagJob.h"

#include <etl/intrusive_forward_list.h>
#include <etl/pool.h>

namespace uds
{
/**
 * Helper class for storing and (re)processing diagnostic requests by using an
 * object pool
 */
class AsyncDiagHelper
: public IAsyncDiagHelper
, private AbstractDiagJob
{
public:
    using StoredRequestPool = ::etl::ipool;

    explicit AsyncDiagHelper(
        StoredRequestPool& storedRequestPool, ::async::ContextType diagContext);

    ::async::ContextType getDiagContext() const override;
    StoredRequest* allocateRequest(
        IncomingDiagConnection& connection,
        uint8_t const* request,
        uint16_t requestLength) override;
    void processAndReleaseRequest(AbstractDiagJob& job, StoredRequest& request) override;

protected:
    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override;

private:
    StoredRequestPool& fStoredRequestPool;
    ::async::ContextType fDiagContext;
};

namespace declare
{
/**
 * Helper class for storing up to a specified number of requests.
 * \tparam N Maximum number of requests that can be stored
 */

template<size_t N>
class AsyncDiagHelper : public ::uds::AsyncDiagHelper
{
public:
    AsyncDiagHelper(::async::ContextType diagContext);

private:
    ::etl::pool<StoredRequest, N> fStoredRequestPool;
};
} // namespace declare

/**
 * Inline implementation.
 */

namespace declare
{
template<size_t N>
inline AsyncDiagHelper<N>::AsyncDiagHelper(::async::ContextType const diagContext)
: ::uds::AsyncDiagHelper(fStoredRequestPool, diagContext), fStoredRequestPool()
{}

} // namespace declare

} // namespace uds
