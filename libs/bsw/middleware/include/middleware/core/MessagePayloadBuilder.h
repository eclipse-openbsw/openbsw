// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/Message.h"
#include "middleware/core/types.h"

#include <etl/iterator.h>
#include <etl/memory.h>
#include <etl/optional.h>
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
     * Chooses internal or external allocation based on whether T is trivially copyable
     * and whether it fits in the internal buffer.
     *
     * \tparam T Payload type
     * \param obj The object to allocate
     * \param msg The message to store the payload in
     * \param numberOfReferences Number of references sharing this payload (1 = unique)
     * \return HRESULT indicating success or failure
     */
    template<typename T>
    [[nodiscard]] HRESULT
    allocate(T const& obj, Message& msg, uint8_t const numberOfReferences = 1U)
    {
        if constexpr (!etl::is_trivially_copyable_v<T>)
        {
            HRESULT ret = HRESULT::CannotAllocatePayload;

            size_t const payloadSize = T::AllocationPolicy::getNeededSize(obj);
            etl::optional<etl::span<uint8_t>> buffer
                = allocateNonTrivialType(payloadSize, msg, numberOfReferences);
            if (buffer.has_value())
            {
                T::AllocationPolicy::serialize(obj, buffer.value());
                ret = HRESULT::Ok;
            }

            return ret;
        }
        else if constexpr (sizeof(T) <= Message::MAX_PAYLOAD_SIZE)
        {
            msg.constructObjectAtPayload(obj);
            return HRESULT::Ok;
        }
        else
        {
            return allocateTrivialType(&obj, sizeof(obj), msg, numberOfReferences);
        }
    }

    /**
     * Specialization for byte spans to copy the span's data contents.
     *
     * \param span Span of bytes to copy into the message
     * \param msg The message to store the payload in
     * \param numberOfReferences Number of shared references
     * \return HRESULT indicating success or failure
     */
    [[nodiscard]] HRESULT allocate(
        etl::span<uint8_t const> const span, Message& msg, uint8_t const numberOfReferences = 1U)
    {
        if (span.size_bytes() <= Message::MAX_PAYLOAD_SIZE)
        {
            msg.copyRawBytesToPayload(span);
            return HRESULT::Ok;
        }
        return allocateTrivialType(span.data(), span.size_bytes(), msg, numberOfReferences);
    }

    /**
     * Reads an object of type T from the content of \p msg.
     *
     * \tparam T The payload type to read
     * \param msg The message containing the payload
     * \return The deserialized object
     */
    template<typename T>
    T readPayload(Message const& msg)
    {
        if constexpr (!etl::is_trivially_copyable_v<T>)
        {
            uint8_t* ptr = getAllocatorPointerFromMessage(msg);
            T obj = T::AllocationPolicy::deserialize(etl::span<uint8_t>(ptr, msg.getPayloadSize()));

            return obj;
        }
        else if constexpr (sizeof(T) <= Message::MAX_PAYLOAD_SIZE)
        {
            return msg.getObjectStoredInPayload<T>();
        }
        else
        {
            uint8_t* ptr = getAllocatorPointerFromMessage(msg);
            return etl::get_object_at<T>(ptr);
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
    static HRESULT allocateTrivialType(
        void const* objPtr, size_t objSize, Message& msg, uint8_t numberOfReferences);

    static etl::optional<etl::span<uint8_t>>
    allocateNonTrivialType(size_t payloadSize, Message& msg, uint8_t numberOfReferences);

    static uint8_t* getAllocatorPointerFromMessage(Message const& msg);

    MessagePayloadBuilder() = default;

    static MessagePayloadBuilder _instance;
};

} // namespace middleware::core
