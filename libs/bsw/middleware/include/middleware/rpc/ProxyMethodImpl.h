/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "middleware/core/Future.h"
#include "middleware/core/Message.h"
#include "middleware/core/ProxyBase.h"
#include "middleware/core/types.h"

#include <etl/delegate.h>
#include <etl/expected.h>
#include <etl/span.h>

#include <cstdint>

namespace middleware::rpc
{

/**
 * \brief Base class providing core future management and request tracking functionality.
 *
 * ProxyMethodImpl manages a pool of Future objects that track pending asynchronous
 * requests. It handles request ID allocation, future lifecycle management, timeout detection,
 * and callback dispatching. Derived classes specialize this for specific argument types and
 * timeout configurations.
 */
class ProxyMethodImpl
{
protected:
    /**
     * \brief Cancels a pending request and frees its associated future.
     *
     * \param requestId The unique identifier of the request to cancel
     * \return HRESULT::Ok if found and cancelled, HRESULT::InstanceNotFound otherwise
     */
    core::HRESULT cancelRequest(uint16_t requestId);

    /**
     * \brief Matches a response message to its originating request and updates future state.
     *
     * \param msg The incoming response message containing the request ID and payload/error
     * \return Pointer to the matched future, or nullptr if not found
     */
    core::Future* futureMatchingRequestId(core::Message const& msg);

    /**
     * \brief Resets all futures to initial state, abandoning any pending requests.
     */
    void freeAll();

    /**
     * \brief Obtains a future from the pool and assigns it a unique request ID.
     *
     * Also records the current system time for timeout tracking.
     *
     * \return Pointer to the allocated future, or nullptr if the pool is exhausted
     */
    core::Future* obtainFuture();

    /**
     * \brief Sends a method call message after validating the allocation result.
     *
     * On allocation or send failure the associated future is cancelled and the message
     * payload is deallocated.
     *
     * \param proxy  The proxy used to send the message
     * \param msg    The message to send (header must already contain the request ID)
     * \param allocationResult Result of the preceding payload allocation
     * \return The request ID on success, or an HRESULT error
     */
    etl::expected<uint16_t, core::HRESULT>
    callMethod(core::ProxyBase const& proxy, core::Message& msg, core::HRESULT allocationResult);

    /**
     * \brief Constructs the base with a span over the caller-owned futures array.
     *
     * \param futures Span referencing the derived class's futures storage
     */
    ProxyMethodImpl(::etl::span<core::Future> const futures)
    : _futures(futures), _currentRequestId(core::INVALID_REQUEST_ID)
    {}

    /**
     * \brief Scans pending requests and triggers timeout callbacks for expired ones.
     *
     * \param timeout Timeout duration in milliseconds
     * \param callback Delegate invoked for each future that has exceeded the timeout
     */
    void updateTimeouts(uint32_t timeout, etl::delegate<void(core::Future&)> const callback);

private:
    uint16_t getNewRequestId();

    ::etl::span<core::Future> _futures;
    uint16_t _currentRequestId;
};

} // namespace middleware::rpc
