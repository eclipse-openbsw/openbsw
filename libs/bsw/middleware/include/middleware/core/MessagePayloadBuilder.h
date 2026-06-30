/********************************************************************************
 * Copyright (c) 2025 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "middleware/core/LoggerApi.h"
#include "middleware/core/Message.h"
#include "middleware/core/types.h"
#include "middleware/memory/AllocatorSelector.h"

#include <etl/iterator.h>
#include <etl/memory.h>
#include <etl/span.h>
#include <etl/type_traits.h>

#include <cstdint>

namespace middleware::core
{

/**
 * Singleton responsible for allocating, reading, and deallocating message payloads.
 * Decides whether a payload should be stored in the message's internal buffer or
 * externally allocated based on the type's properties and size.
 */
class MessagePayloadBuilder final
{
public:
    ~MessagePayloadBuilder()                                             = default;
    MessagePayloadBuilder(MessagePayloadBuilder const& other)            = delete;
    MessagePayloadBuilder(MessagePayloadBuilder&& other)                 = delete;
    MessagePayloadBuilder& operator=(MessagePayloadBuilder const& other) = delete;
    MessagePayloadBuilder& operator=(MessagePayloadBuilder&& other)      = delete;

    /**
     * Allocates space for \p obj in the message payload.
     * Stores \p obj in the message's internal buffer when it fits, otherwise copy-constructs
     * it into externally allocated memory.
     *
     * \tparam T Payload type (must be copy-constructible)
     * \param obj The object to allocate
     * \param msg The message to store the payload in
     * \param numberOfReferences Number of references sharing this payload (1 = unique)
     * \return HRESULT indicating success or failure
     */
    template<typename T>
    [[nodiscard]] HRESULT
    allocate(T const& obj, Message& msg, uint8_t const numberOfReferences = 1U)
    {
        static_assert(etl::is_copy_constructible_v<T>, "T must be copy-constructible");
        static_assert(!etl::is_span_v<T>, "Use the span overload to allocate bytes");

        if constexpr (sizeof(T) <= Message::MAX_PAYLOAD_SIZE)
        {
            msg.constructObjectAtPayload(obj);
            return HRESULT::Ok;
        }
        else
        {
            return emplaceExternalPayload(obj, msg, numberOfReferences);
        }
    }

    /**
     * Specialization for byte spans to copy the span's data contents.
     *
     * \param src Span of bytes to copy into the message
     * \param msg The message to store the payload in
     * \param numberOfReferences Number of shared references
     * \return HRESULT indicating success or failure
     */
    [[nodiscard]] static HRESULT
    allocate(etl::span<uint8_t const> src, Message& msg, uint8_t numberOfReferences = 1U);

    /**
     * Reads an object of type T from the content of \p msg.
     *
     * Returns a const reference directly into the middleware-managed buffer. The caller must
     * consume the reference before the message is deallocated, as the middleware reclaims the
     * buffer immediately after dispatch.
     *
     * \tparam T The payload type to read
     * \param msg The message containing the payload
     * \return A const reference to the object stored in the payload
     */
    template<typename T>
    static T const& readPayload(Message const& msg)
    {
        static_assert(!etl::is_span_v<T>, "Use readRawPayload() to read bytes");

        if constexpr (sizeof(T) <= Message::MAX_PAYLOAD_SIZE)
        {
            return msg.getObjectStoredInPayload<T>();
        }
        else
        {
            return etl::get_object_at<T const>(getAllocatorPointerFromMessage(msg));
        }
    }

    /**
     * Reads the raw payload from the message as a span of bytes.
     *
     * \param msg The message containing the payload
     * \return A const span providing read-only access to the payload bytes
     */
    static etl::span<uint8_t const> readRawPayload(Message const& msg)
    {
        etl::span<uint8_t const> rawPayload;

        if (!msg.hasExternalPayload())
        {
            rawPayload = etl::span<uint8_t const>(
                &msg.getObjectStoredInPayload<uint8_t>(), msg.getPayloadSize());
        }
        else
        {
            rawPayload = etl::span<uint8_t const>(
                getAllocatorPointerFromMessage(msg), msg.getPayloadSize());
        }

        return rawPayload;
    }

    /**
     * Releases any external resources if \p msg contains an externally stored payload.
     *
     * \param msg The message containing the payload
     */
    static void deallocate(Message const& msg);

    /**
     * Returns the singleton MessagePayloadBuilder instance.
     *
     * \return MessagePayloadBuilder& The singleton instance
     */
    static MessagePayloadBuilder& getInstance() { return _instance; }

private:
    /**
     * Allocates space on the middleware allocators for a byte span by copying bytes.
     *
     * \param src Span of bytes to copy
     * \param msg Reference to the middleware message
     * \param numberOfReferences Number of references to the allocation
     * \return HRESULT
     */
    static HRESULT allocAndCopyBytesToExternalPayload(
        etl::span<uint8_t const> src, Message& msg, uint8_t numberOfReferences);

    static uint8_t* getAllocatorPointerFromMessage(Message const& msg);

    /**
     * Emplaces an object of type T into externally allocated memory using placement new.
     *
     * \tparam T Must be copy-constructible
     * \param obj Object to copy-construct into the allocation
     * \param msg Reference to the middleware message
     * \param numberOfReferences Number of shared references
     * \return HRESULT
     */
    template<typename T>
    [[nodiscard]] HRESULT
    emplaceExternalPayload(T const& obj, Message& msg, uint8_t const numberOfReferences)
    {
        static_assert(etl::is_copy_constructible<T>::value, "T must have a copy constructor!");

        HRESULT ret = HRESULT::CannotAllocatePayload;

        uint16_t const sid = msg.getHeader().serviceId;
        uint8_t* const buffer
            = msg.isEvent() ? memory::getAllocSharedFunction(sid)(sizeof(T), numberOfReferences)
                            : memory::getAllocFunction(sid)(sizeof(T));

        if (buffer != nullptr)
        {
            etl::construct_object_at(buffer, obj);
            auto const offset = static_cast<ptrdiff_t>(
                etl::distance(memory::getRegionStartFunction(sid)(), buffer));
            msg.setExternalPayload(offset, static_cast<uint32_t>(sizeof(T)));
            ret = HRESULT::Ok;
        }
        else
        {
            logger::logAllocationFailure(
                logger::LogLevel::Error,
                logger::Error::Allocation,
                ret,
                msg,
                static_cast<uint32_t>(sizeof(T)));
        }

        return ret;
    }

    MessagePayloadBuilder() = default;

    static MessagePayloadBuilder _instance;
};

} // namespace middleware::core
