// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/Message.h"
#include "middleware/core/SkeletonBase.h"
#include "middleware/core/types.h"

#include <etl/vector.h>

#include <cstdint>

namespace middleware
{
namespace core
{

/**
 * Base class for managing skeleton response slots.
 * Tracks pending responses and provides send/cancel operations.
 */
class ResponseBufferBase
{
public:
    struct SkeletonResponseInfo
    {
        uint8_t addressId{INVALID_ADDRESS_ID};
        uint8_t targetClusterId{INVALID_CLUSTER_ID};
        uint16_t requestId{INVALID_REQUEST_ID};
    };

    /**
     * Constructs a response buffer for managing skeleton responses.
     *
     * \param skeleton Reference to the skeleton instance
     * \param responses Vector for storing pending response information
     */
    ResponseBufferBase(SkeletonBase const& skeleton, etl::ivector<SkeletonResponseInfo>& responses);

    /**
     * Allocates a new response slot if capacity is available.
     *
     * \param addressId Unique address ID of the target proxy
     * \param targetClusterId Cluster ID of the target proxy
     * \param requestId Request identifier from the original request
     * \return Pointer to response info slot, or nullptr if none available
     */
    SkeletonResponseInfo*
    getAvailableResponse(uint8_t addressId, uint8_t targetClusterId, uint16_t requestId);

    /**
     * Cancels a pending response and frees its slot.
     *
     * \param response Response information to cancel
     * \return HRESULT Result code
     */
    HRESULT cancelResponse(SkeletonResponseInfo& response);

    /**
     * Validates if a response pointer refers to a current response slot.
     *
     * \param iterator Pointer to response information to validate
     * \return true if pointer refers to a slot in the buffer
     */
    bool isResponseIteratorValid(SkeletonResponseInfo* iterator) const;

protected:
    HRESULT sendResponse(Message& msg, SkeletonResponseInfo& response, bool handleResponseFailure);

    Message generateResponseMessageHeader(
        SkeletonResponseInfo const& responseInfo, uint16_t const memberId) const
    {
        return Message::createResponse(
            _skeleton.getServiceId(),
            memberId,
            responseInfo.requestId,
            _skeleton.getInstanceId(),
            _skeleton.getSourceClusterId(),
            responseInfo.targetClusterId,
            responseInfo.addressId);
    }

    void makeResponseAvailable(SkeletonResponseInfo& response);

private:
    SkeletonBase const& _skeleton;
    etl::ivector<SkeletonResponseInfo>& _responses;
};

} // namespace core
} // namespace middleware
