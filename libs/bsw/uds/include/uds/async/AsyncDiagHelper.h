// Copyright 2024 Accenture.

#ifndef GUARD_5FA75AF6_D0BD_4A3B_A25D_C6C18E5A24BD
#define GUARD_5FA75AF6_D0BD_4A3B_A25D_C6C18E5A24BD

#include "uds/async/IAsyncDiagHelper.h"
#include "uds/base/AbstractDiagJob.h"

#include <estd/forward_list.h>
#include <estd/object_pool.h>

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
    using StoredRequestPool = ::estd::object_pool<StoredRequest>;

    explicit AsyncDiagHelper(StoredRequestPool& storedRequestPool);

    StoredRequest* allocateRequest(
        IncomingDiagConnection& connection,
        uint8_t const* request,
        uint16_t requestLength) override;
    void processAndReleaseRequest(AbstractDiagJob& job, StoredRequest& request) override;

protected:
    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override;

private:
    StoredRequestPool& fStoredRequestPool;
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
    AsyncDiagHelper();

private:
    ::estd::declare::object_pool<StoredRequest, N> fStoredRequestPool;
};
} // namespace declare

/**
 * Inline implementation.
 */

namespace declare
{
template<size_t N>
inline AsyncDiagHelper<N>::AsyncDiagHelper()
: ::uds::AsyncDiagHelper(fStoredRequestPool), fStoredRequestPool()
{}

} // namespace declare

} // namespace uds

#endif // GUARD_5FA75AF6_D0BD_4A3B_A25D_C6C18E5A24BD
