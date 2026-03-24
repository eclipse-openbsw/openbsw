// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/Future.h"
#include "middleware/core/Message.h"
#include "middleware/core/types.h"

#include <etl/delegate.h>
#include <etl/expected.h>
#include <etl/span.h>
#include <etl/vector.h>

#include <cstddef>
#include <cstdint>

namespace middleware
{
namespace core
{

/**
 * Base class providing core future management and request tracking functionality.
 * FutureDispatcherBase manages a pool of Future objects that track pending asynchronous
 * requests. It handles request ID allocation, future lifecycle management, timeout detection,
 * and callback dispatching. Derived classes specialize this for specific argument types and
 * timeout configurations.
 */
class FutureDispatcherBase
{
public:
    FutureDispatcherBase(FutureDispatcherBase const&)            = delete;
    FutureDispatcherBase& operator=(FutureDispatcherBase const&) = delete;
    FutureDispatcherBase(FutureDispatcherBase&&)                 = delete;
    FutureDispatcherBase& operator=(FutureDispatcherBase&&)      = delete;

    /**
     * Cancels a pending request and frees its associated future.
     *
     * \param requestId The unique identifier of the request to cancel
     * \return HRESULT::Ok if found and cancelled, HRESULT::InstanceNotFound otherwise
     */
    HRESULT cancelRequest(uint16_t requestId);

    /**
     * Matches a response message to its originating request and updates future state.
     *
     * \param msg The incoming response message containing the request ID and payload/error
     * \return Pointer to the matched future, or nullptr if not found
     */
    Future* futureMatchingRequestId(Message const& msg);

    /**
     * Resets all futures to initial state, abandoning any pending requests.
     */
    void freeAll();

protected:
    using CallbackDispatcher = etl::delegate<void(Message const&, Future&)>;

    /**
     * Allocates a future and generates a unique request ID.
     *
     * \param requestId Output parameter receiving the allocated request ID
     * \return etl::optional containing pointer to allocated future, or etl::nullopt
     */
    etl::optional<Future*> obtainRequestId(uint16_t& requestId);

    FutureDispatcherBase(etl::span<Future> const futures, CallbackDispatcher const cbk)
    : _futures(futures), _callback(cbk), _currentRequestId(INVALID_REQUEST_ID)
    {}

    ~FutureDispatcherBase() = default;

    /**
     * Scans pending requests and triggers timeout callbacks for expired ones.
     *
     * \param now Current system time in milliseconds
     * \param timeout Timeout duration in milliseconds
     */
    void updateTimeouts(uint32_t now, uint32_t timeout);

private:
    uint16_t getRequestId();

    etl::span<Future> _futures;
    CallbackDispatcher _callback;
    uint16_t _currentRequestId;
};

} // namespace core
} // namespace middleware
