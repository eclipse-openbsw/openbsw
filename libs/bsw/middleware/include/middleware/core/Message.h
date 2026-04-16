// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/types.h"

#include <etl/array.h>
#include <etl/memory.h>
#include <etl/span.h>
#include <etl/utility.h>

#include <cstddef>
#include <cstdint>

namespace middleware::core
{

/**
 * Message object that is used for communication between proxies and skeletons.
 * This object has 64 bytes in total which are distributed between its private members.
 * These consist of the following members:
 * - header, which is similar to the SOME/IP's message ID and request ID members;
 * - some dispatching information like the source cluster from where the message needs to go, an
 * address ID that is unique to each proxy instance, the message flags and the size of the payload;
 * - a payload which is a union of an internal buffer, an external handle that contains
 *   information to where the actual payload might be stored or an error value.
 */
class Message final
{
public:
    enum class MessageType : uint8_t
    {
        Request              = 1U << 0U,
        FireAndForgetRequest = 1U << 1U,
        Response             = 1U << 2U,
        Event                = 1U << 3U,
        Error                = 1U << 4U
    };

    /** The message's header. */
    struct Header
    {
        uint16_t serviceId;
        uint16_t memberId;
        uint16_t requestId;
        uint16_t serviceInstanceId;
        uint8_t srcClusterId;
        uint8_t tgtClusterId;
        uint8_t addressId;
        MessageType messageType : 7;
        bool hasExternalPayload : 1;
    };

    static constexpr size_t MAX_MESSAGE_SIZE = 64U;
    static constexpr size_t MAX_PAYLOAD_SIZE = MAX_MESSAGE_SIZE - sizeof(Header) - sizeof(uint32_t);

    /**
     * The payload of a message, which can be either an internal buffer, an offset or an error
     * value. The offset member of this union is offset of the memory section where the data is
     * stored. Whenever the data that needs to be sent is bigger than MAX_PAYLOAD_SIZE, the data
     * will be allocated externally and the message's payload set to this offset value.
     */
    union PayloadType
    {
        etl::array<uint8_t, MAX_PAYLOAD_SIZE> internalBuffer{};
        ptrdiff_t offset;
        ErrorState error;
    };

    /**
     * Default c'tor must remain empty.
     * \remark In case a core is initialized after another core has already sent a message over
     * a queue, the default c'tor of all messages is called again, thus invalidating the first
     * core's message payload. As such the constructor needs to be empty in order to not do any
     * work.
     */
    Message() {}

    ~Message()                                 = default;
    Message(Message const& other)              = default;
    Message& operator=(Message const& other) & = default;
    Message(Message&& other)                   = default;
    Message& operator=(Message&& other) &      = default;

    /** Returns a const reference to the message header. */
    Header const& getHeader() const { return _header; }

    /**
     * Create a request message that originates from a proxy and is targetted to a specific
     * skeleton.
     */
    static constexpr Message createRequest(
        uint16_t const serviceId,
        uint16_t const memberId,
        uint16_t const requestId,
        uint16_t const serviceInstanceId,
        uint8_t const srcClusterId,
        uint8_t const tgtClusterId,
        uint8_t const addressId)
    {
        Header const header{
            serviceId,
            memberId,
            requestId,
            serviceInstanceId,
            srcClusterId,
            tgtClusterId,
            addressId,
            MessageType::Request,
            false};

        return {header};
    }

    /**
     * Create a fireAndForget request message that originates from a proxy and is targetted
     * to a specific skeleton.
     */
    static constexpr Message createFireAndForgetRequest(
        uint16_t const serviceId,
        uint16_t const memberId,
        uint16_t const serviceInstanceId,
        uint8_t const srcClusterId,
        uint8_t const tgtClusterId)
    {
        Header const header{
            serviceId,
            memberId,
            INVALID_REQUEST_ID,
            serviceInstanceId,
            srcClusterId,
            tgtClusterId,
            INVALID_ADDRESS_ID,
            MessageType::FireAndForgetRequest,
            false};

        return {header};
    }

    /**
     * Create a response message that originates from a skeleton and is targetted to a unique
     * proxy.
     */
    static constexpr Message createResponse(
        uint16_t const serviceId,
        uint16_t const memberId,
        uint16_t const requestId,
        uint16_t const serviceInstanceId,
        uint8_t const srcClusterId,
        uint8_t const tgtClusterId,
        uint8_t const addressId)
    {
        Header const header{
            serviceId,
            memberId,
            requestId,
            serviceInstanceId,
            srcClusterId,
            tgtClusterId,
            addressId,
            MessageType::Response,
            false};

        return {header};
    }

    /**
     * Create an event message without source cluster and target cluster IDs set.
     * \remark After calling this method, you need to manually set the target cluster ID. This
     * allows to use the same message to be sent to several clusters by just adapting the cluster
     * ID.
     */
    static constexpr Message createEvent(
        uint16_t const serviceId,
        uint16_t const memberId,
        uint16_t const serviceInstanceId,
        uint8_t const srcClusterId)
    {
        Header const header{
            serviceId,
            memberId,
            INVALID_REQUEST_ID,
            serviceInstanceId,
            srcClusterId,
            INVALID_CLUSTER_ID,
            INVALID_ADDRESS_ID,
            MessageType::Event,
            false};

        return {header};
    }

    /**
     * Create an error response message that originates from a skeleton and is targetted to a
     * unique proxy, and sets the payload with value \param error.
     */
    static constexpr Message createErrorResponse(
        uint16_t const serviceId,
        uint16_t const memberId,
        uint16_t const requestId,
        uint16_t const serviceInstanceId,
        uint8_t const srcClusterId,
        uint8_t const tgtClusterId,
        uint8_t const addressId,
        ErrorState const error)
    {
        Header const header{
            serviceId,
            memberId,
            requestId,
            serviceInstanceId,
            srcClusterId,
            tgtClusterId,
            addressId,
            MessageType::Error,
            false};

        Message msg{header};
        msg._payload.error = error;
        msg._size          = sizeof(ErrorState);

        return msg;
    }

    /** Returns true if the message is a request. */
    bool isRequest() const { return _header.messageType == MessageType::Request; }

    /** Returns true if the message is a fire-and-forget request. */
    bool isFireAndForgetRequest() const
    {
        return _header.messageType == MessageType::FireAndForgetRequest;
    }

    /** Returns true if the message is a response. */
    bool isResponse() const { return _header.messageType == MessageType::Response; }

    /** Returns true if the message is an error response. */
    bool isError() const { return _header.messageType == MessageType::Error; }

    /** Returns true if the message is an event. */
    bool isEvent() const { return _header.messageType == MessageType::Event; }

    /** Returns true if the message contains an external payload. */
    bool hasExternalPayload() const { return _header.hasExternalPayload; }

    /**
     * Returns the size of the payload stored in the message.
     * This value is set when an object is constructed in the payload using
     * constructObjectAtPayload or setExternalPayload.
     *
     * \return uint32_t The size in bytes of the payload
     */
    uint32_t getPayloadSize() const { return _size; }

    /** Returns the ErrorState if the message has an error, otherwise ErrorState::NoError. */
    ErrorState getErrorState() const { return isError() ? _payload.error : ErrorState::NoError; }

    /**
     * Gets a constant reference of the object that is stored inside the payload's internal
     * buffer.
     * \remark This method assumes that the user has checked that the message contains the
     * payload in its internal buffer, and not an offset or an error value.
     *
     * \tparam T the generic type which must be copy-constructible and have a size less than the
     * internal payload's size.
     * \return constexpr const T&
     */
    template<typename T>
    constexpr T const& getObjectStoredInPayload() const
    {
        static_assert(
            sizeof(T) <= MAX_PAYLOAD_SIZE,
            "Size of type T must be less than or equal to MAX_PAYLOAD_SIZE");
        static_assert(etl::is_copy_constructible<T>::value, "T must have a copy constructor!");

        return etl::get_object_at<T const>(_payload.internalBuffer.data());
    }

    /**
     * Constructs a copy of \param obj inside the payload's internal buffer.
     *
     * \tparam T the generic type which must be copy-constructible and have a size less than the
     * internal payload's size.
     * \param obj
     */
    template<typename T>
    constexpr void constructObjectAtPayload(T const& obj)
    {
        static_assert(
            sizeof(T) <= MAX_PAYLOAD_SIZE, "Size of type T must be smaller than MAX_PAYLOAD_SIZE!");
        static_assert(etl::is_copy_constructible<T>::value, "T must have a copy constructor!");

        _header.hasExternalPayload = false;
        etl::construct_object_at(_payload.internalBuffer.data(), obj);
        _size = sizeof(T);
    }

    /**
     * Copies raw bytes into the payload's internal buffer.
     *
     * \param span Span of const bytes to copy
     */
    void copyRawBytesToPayload(etl::span<uint8_t const> const span)
    {
        _header.hasExternalPayload = false;
        size_t const dataSize
            = span.size_bytes() < MAX_PAYLOAD_SIZE ? span.size_bytes() : MAX_PAYLOAD_SIZE;
        etl::mem_copy(span.data(), dataSize, _payload.internalBuffer.data());
        _size = static_cast<uint32_t>(dataSize);
    }

    /**
     * Gets the external payload information as a pair of offset and size.
     * \remark This method assumes that the user has checked that the message
     * contains an external payload.
     *
     * \return etl::pair<ptrdiff_t, uint32_t>  offset and size
     */
    etl::pair<ptrdiff_t, uint32_t> getExternalPayload() const
    {
        return etl::make_pair(_payload.offset, _size);
    }

    /**
     * Set the payload as an external type which will contain information to where the
     * actual message's payload is stored.
     *
     * \param offset Offset from the beginning of the memory region
     * \param size Size of the payload
     */
    void setExternalPayload(ptrdiff_t const offset, uint32_t const size)
    {
        _header.hasExternalPayload = true;
        etl::construct_object_at<ptrdiff_t>(&_payload.offset, offset);
        _size = size;
    }

private:
    constexpr Message(Header const& header) : _header(header), _size(), _payload() {}

    /** The message header containing the service, method, request and instance ids. */
    Header _header;
    /** The size of the payload stored in the message. */
    uint32_t _size;
    /** The message payload which can either store some data constructed in place, an external
     * handle pointing to where the actual data is stored, or an error value. */
    mutable PayloadType _payload;
};

static_assert(
    Message::MAX_MESSAGE_SIZE == sizeof(Message),
    "size of middleware::core::Message must be the same as cache line alignment!");

} // namespace middleware::core
