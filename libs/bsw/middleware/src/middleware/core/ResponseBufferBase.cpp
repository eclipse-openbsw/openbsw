// Copyright 2025 BMW AG

#include "middleware/core/ResponseBufferBase.h"

#include "middleware/core/Message.h"
#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/SkeletonBase.h"
#include "middleware/core/types.h"

#include <etl/algorithm.h>
#include <etl/vector.h>

namespace middleware
{
namespace core
{

ResponseBufferBase::ResponseBufferBase(
    SkeletonBase const& skeleton, etl::ivector<SkeletonResponseInfo>& responses)
: _skeleton(skeleton), _responses(responses)
{}

ResponseBufferBase::SkeletonResponseInfo* ResponseBufferBase::getAvailableResponse(
    uint8_t const addressId, uint8_t const targetClusterId, uint16_t const requestId)
{
    SkeletonResponseInfo* availableResponse = nullptr;
    if (!_responses.full())
    {
        SkeletonResponseInfo newResponse{addressId, targetClusterId, requestId};
        availableResponse = &_responses.emplace_back(newResponse);
    }

    return availableResponse;
}

void ResponseBufferBase::makeResponseAvailable(SkeletonResponseInfo& response)
{
    _responses.erase(&response);
}

HRESULT ResponseBufferBase::cancelResponse(SkeletonResponseInfo& response)
{
    HRESULT ret = HRESULT::ResponseBufferFutureNotFound;
    if (isResponseIteratorValid(&response))
    {
        makeResponseAvailable(response);
        ret = HRESULT::Ok;
    }

    return ret;
}

bool ResponseBufferBase::isResponseIteratorValid(SkeletonResponseInfo* const iterator) const
{
    SkeletonResponseInfo* ret = etl::find_if(
        _responses.begin(),
        _responses.end(),
        [&iterator](SkeletonResponseInfo const& response) { return &response == iterator; });

    return ret != _responses.end();
}

HRESULT ResponseBufferBase::sendResponse(
    Message& msg, SkeletonResponseInfo& response, bool const handleResponseFailure)
{
    HRESULT const res = _skeleton.sendMessage(msg);
    if (res == HRESULT::Ok)
    {
        makeResponseAvailable(response);
    }
    else
    {
        MessagePayloadBuilder::deallocate(msg);
        if (handleResponseFailure)
        {
            static_cast<void>(cancelResponse(response));
        }
    }

    return res;
}

} // namespace core
} // namespace middleware
