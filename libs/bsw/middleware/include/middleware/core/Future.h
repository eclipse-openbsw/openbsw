// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/types.h"

#include <cstdint>

namespace middleware
{
namespace core
{

/**
 * Represents the state and tracking information for an asynchronous request.
 * A Future tracks a single pending request-response interaction in the middleware. It stores
 * the unique request identifier, timing information for timeout detection, and the current
 * state of the request lifecycle. Futures are pooled and reused across multiple requests to
 * avoid dynamic allocation.
 */
struct Future
{
    /**
     * Represents the lifecycle state of an asynchronous request.
     */
    enum class State : uint8_t
    {
        Invalid            = 0U, //!< Future slot is unallocated and available for reuse
        Pending            = 1U, //!< Request sent, awaiting response from remote service
        Ready              = 2U, //!< Response received successfully with valid payload
        Timeout            = 3U, //!< Request exceeded configured timeout duration without response
        UserError          = 4U, //!< Remote service returned an application-level error
        ServiceBusy        = 5U, //!< Service rejected request due to insufficient resources
        ServiceNotFound    = 6U, //!< Target service is not registered or reachable
        SerializationError = 7U, //!< Failed to serialize request payload
        DeserializationError = 8U, //!< Failed to deserialize response payload
        CouldNotDeliverError = 9U  //!< Transport layer failed (queue full, network unreachable)
    };

    uint32_t callerTimestamp;
    uint16_t requestId;
    State state;
};

} // namespace core
} // namespace middleware
