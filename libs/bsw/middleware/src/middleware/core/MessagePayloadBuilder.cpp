/********************************************************************************
 * Copyright (c) 2025 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/core/MessagePayloadBuilder.h"

#include "middleware/core/LoggerApi.h"
#include "middleware/core/Message.h"
#include "middleware/core/types.h"
#include "middleware/logger/Logger.h"
#include "middleware/memory/AllocatorSelector.h"

#include <etl/iterator.h>
#include <etl/memory.h>
#include <etl/span.h>

#include <cstdint>

namespace middleware::core
{

MessagePayloadBuilder MessagePayloadBuilder::_instance{};

HRESULT MessagePayloadBuilder::allocAndCopyBytesToExternalPayload(
    etl::span<uint8_t const> const src, Message& msg, uint8_t const numberOfReferences)
{
    uint16_t const sid  = msg.getHeader().serviceId;
    uint8_t* const dest = msg.isEvent()
                              ? memory::getAllocSharedFunction(sid)(src.size(), numberOfReferences)
                              : memory::getAllocFunction(sid)(src.size());

    if (dest == nullptr)
    {
        logger::logAllocationFailure(
            logger::LogLevel::Error,
            logger::Error::Allocation,
            HRESULT::CannotAllocatePayload,
            msg,
            static_cast<uint32_t>(src.size()));
        return HRESULT::CannotAllocatePayload;
    }

    etl::mem_copy(src.data(), src.size(), dest);
    auto const offset
        = static_cast<ptrdiff_t>(etl::distance(memory::getRegionStartFunction(sid)(), dest));
    msg.setExternalPayload(offset, static_cast<uint32_t>(src.size()));

    return HRESULT::Ok;
}

uint8_t* MessagePayloadBuilder::getAllocatorPointerFromMessage(Message const& msg)
{
    uint16_t const sid               = msg.getHeader().serviceId;
    ptrdiff_t const offset           = msg.getExternalPayload().first;
    uint8_t* const externalBufferPtr = etl::next(memory::getRegionStartFunction(sid)(), offset);

    return externalBufferPtr;
}

HRESULT MessagePayloadBuilder::allocate(
    etl::span<uint8_t const> const src, Message& msg, uint8_t const numberOfReferences)
{
    if (src.size_bytes() <= Message::MAX_PAYLOAD_SIZE)
    {
        msg.copyRawBytesToPayload(src);
        return HRESULT::Ok;
    }
    return allocAndCopyBytesToExternalPayload(src, msg, numberOfReferences);
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
