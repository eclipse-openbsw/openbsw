// Copyright 2025 BMW AG

#include "middleware/core/MessagePayloadBuilder.h"

#include "middleware/core/LoggerApi.h"
#include "middleware/core/Message.h"
#include "middleware/core/types.h"
#include "middleware/logger/Logger.h"
#include "middleware/memory/AllocatorSelector.h"

#include <etl/iterator.h>
#include <etl/optional.h>
#include <etl/span.h>

#include <cstdint>
#include <cstring>

namespace middleware::core
{

MessagePayloadBuilder MessagePayloadBuilder::_instance{};

HRESULT MessagePayloadBuilder::allocateTrivialType(
    void const* const objPtr, size_t const objSize, Message& msg, uint8_t const numberOfReferences)
{
    HRESULT ret = HRESULT::CannotAllocatePayload;

    uint16_t const sid    = msg.getHeader().serviceId;
    uint8_t* const buffer = msg.isEvent()
                                ? memory::getAllocSharedFunction(sid)(objSize, numberOfReferences)
                                : memory::getAllocFunction(sid)(objSize);

    if (buffer != nullptr)
    {
        std::memcpy(buffer, objPtr, objSize);
        auto const offset
            = static_cast<ptrdiff_t>(etl::distance(memory::getRegionStartFunction(sid)(), buffer));
        msg.setExternalPayload(offset, static_cast<uint32_t>(objSize));
        ret = HRESULT::Ok;
    }
    else
    {
        logger::logAllocationFailure(
            logger::LogLevel::Error,
            logger::Error::Allocation,
            ret,
            msg,
            static_cast<uint32_t>(objSize));
    }

    return ret;
}

etl::optional<etl::span<uint8_t>> MessagePayloadBuilder::allocateNonTrivialType(
    size_t const payloadSize, Message& msg, uint8_t const numberOfReferences)
{
    etl::optional<etl::span<uint8_t>> ret{};

    uint16_t const sid = msg.getHeader().serviceId;
    uint8_t* const buffer
        = msg.isEvent() ? memory::getAllocSharedFunction(sid)(payloadSize, numberOfReferences)
                        : memory::getAllocFunction(sid)(payloadSize);
    if (buffer != nullptr)
    {
        auto const offset
            = static_cast<ptrdiff_t>(etl::distance(memory::getRegionStartFunction(sid)(), buffer));
        msg.setExternalPayload(offset, static_cast<uint32_t>(payloadSize));
        ret.emplace(buffer, payloadSize);
    }
    else
    {
        logger::logAllocationFailure(
            logger::LogLevel::Error,
            logger::Error::Allocation,
            HRESULT::CannotAllocatePayload,
            msg,
            static_cast<uint32_t>(payloadSize));
    }

    return ret;
}

uint8_t* MessagePayloadBuilder::getAllocatorPointerFromMessage(Message const& msg)
{
    uint16_t const sid               = msg.getHeader().serviceId;
    ptrdiff_t const offset           = msg.getExternalPayload().first;
    uint8_t* const externalBufferPtr = etl::next(memory::getRegionStartFunction(sid)(), offset);

    return externalBufferPtr;
}

void MessagePayloadBuilder::deallocate(Message const& msg)
{
    uint16_t const sid = msg.getHeader().serviceId;
    if (msg.hasExternalPayload())
    {
        uint8_t* externalPtr       = getAllocatorPointerFromMessage(msg);
        uint32_t const payloadSize = msg.getPayloadSize();
        bool res                   = false;
        if (msg.isEvent())
        {
            res = middleware::memory::getDeallocSharedFunction(sid)(externalPtr, payloadSize);
        }
        else
        {
            res = middleware::memory::getDeallocFunction(sid)(externalPtr);
        }

        if (!res)
        {
            logger::logAllocationFailure(
                logger::LogLevel::Error,
                logger::Error::Deallocation,
                HRESULT::CannotDeallocatePayload,
                msg,
                payloadSize);
        }
    }
}

} // namespace middleware::core
