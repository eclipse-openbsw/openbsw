// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/Message.h"
#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/ResponseBufferBase.h"
#include "middleware/core/types.h"

#include <etl/optional.h>
#include <etl/type_traits.h>
#include <etl/vector.h>

#include <cstdint>

namespace middleware::core
{

/**
 * Compile-time traits describing a response's type and member ID.
 *
 * \tparam T The response payload type
 * \tparam MemberIdValue The member ID for this response
 */
template<typename T, uint16_t MemberIdValue>
struct ResponseTraits
{
    using ResponseType                         = T;
    static constexpr uint16_t METHOD_MEMBER_ID = MemberIdValue;
};

/// Out-of-class definition required for ODR-use in C++14.
template<typename T, uint16_t MemberIdValue>
constexpr uint16_t ResponseTraits<T, MemberIdValue>::METHOD_MEMBER_ID;

/**
 * Templated response buffer for skeleton responses.
 * Manages a fixed-capacity pool of response slots and handles payload allocation
 * and sending.
 *
 * \tparam Traits ResponseTraits describing the response type and member ID
 * \tparam RESPONSE_LIMIT Maximum number of concurrent pending responses
 */
template<typename Traits, uint16_t RESPONSE_LIMIT>
class ResponseBuffer : public ResponseBufferBase
{
public:
    using Base               = ResponseBufferBase;
    using ResponseTraitsType = Traits;
    using ResponseType       = typename Traits::ResponseType;
    using ResponseContainer  = etl::vector<Base::SkeletonResponseInfo, RESPONSE_LIMIT>;

    ResponseBuffer(SkeletonBase const& skeleton) : Base(skeleton, _responses) {}

    /**
     * Sends a response for a previously allocated response slot.
     *
     * \param response Response slot info
     * \param result Response payload to send
     * \param handleResponseFailure True to cancel the response on send failure
     * \return HRESULT Result code
     */
    HRESULT respond(
        SkeletonResponseInfo& response,
        ResponseType const& result,
        bool const handleResponseFailure = true)
    {
        auto ret = HRESULT::ResponseBufferFutureNotFound;

        if (Base::isResponseIteratorValid(&response))
        {
            Message msg = Base::generateResponseMessageHeader(response, Traits::METHOD_MEMBER_ID);
            ret         = MessagePayloadBuilder::getInstance().allocate(result, msg);
            if (ret == HRESULT::Ok)
            {
                ret = Base::sendResponse(msg, response, handleResponseFailure);
            }
        }

        return ret;
    }

private:
    ResponseContainer _responses;
};

} // namespace middleware::core
