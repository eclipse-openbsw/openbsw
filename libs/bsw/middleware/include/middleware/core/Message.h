// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/types.h"

#include <etl/array.h>
#include <etl/memory.h>

#include <cstddef>
#include <cstdint>

namespace middleware
{
namespace core
{

/**
 * \brief Message object that is used for communication between proxies and skeletons.
 * \details This object has 32 bytes in total which are distributed between its private members.
 * These consist of the following members:
 * - header, which is similar to the SOME/IP's message ID and request ID members;
 * - Additionally the header has some dispatching information like the source cluster from where the
 * message originates, the target cluster to which the message needs to go and an address ID that is
 * unique to each proxy instance;
 * - a payload which is a union of a buffer, and external handle that contains information to where
 * the actual payload might be stored or an error value.
 */
class Message
{
public:
    static constexpr size_t MAX_PAYLOAD_SIZE = 32U;

    /**
     * \brief The message's header.
     *
     */
    struct Header
    {
        uint16_t serviceId;
        uint16_t memberId;
        uint16_t requestId;
        uint16_t serviceInstanceId;
        uint8_t srcClusterId;
        uint8_t tgtClusterId;
        uint8_t addressId;
        uint8_t flags;
    };

    /**
     * \brief An object that contains information to where the data is stored.
     * \details This object will be one of the possible types of the payload union. Whenever the
     * data that needs to be sent is biffer than MAX_PAYLOAD_SIZE, the data must be allocated
     * externally and the message's payload set to this object. This object has an offset from
     * the beginning of the memory section where the data was stored, and the size of the data.
     *
     */
    struct ExternalHandle
    {
        ptrdiff_t offset; ///< The offset, from the beginning of the memory region dedicated for
                          ///< storing middleware data, where the message's payload is stored.
        size_t size;      ///< The size of the payload that is stored.
    };

    union PayloadType
    {
        constexpr PayloadType() : internalBuffer() {}

        etl::array<uint8_t, MAX_PAYLOAD_SIZE>
            internalBuffer;            ///< A buffer that can store some data in place.
        ExternalHandle externalHandle; ///< An object that contains information to where the data is
                                       ///< located in memory.
        ErrorState error; ///< An error value, which gives information of what error might have
                          ///< occurred during the communication.
    };

    ~Message()                                 = default;
    Message(Message const& other)              = default;
    Message& operator=(Message const& other) & = default;
    Message(Message&& other)                   = default;
    Message& operator=(Message&& other) &      = default;

    /**
     * \brief Get a constant reference to the message's header.
     *
     * \return const Header&
     */
    Header const& getHeader() const { return _header; }

    /**
     * \brief Set the target cluster id.
     * \details This method can be useful to send the same message to several recipients,
     * by just changing the target cluster id.
     *
     * \param clusterId
     */
    void setTargetClusterId(uint8_t const clusterId) { _header.tgtClusterId = clusterId; }

    /**
     * \brief Create a request message that originates from a proxy and is targetted to a specific
     * skeleton.
     *
     * \param header reference to the message header
     * \return created message
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
        Header header{
            serviceId,
            memberId,
            requestId,
            serviceInstanceId,
            srcClusterId,
            tgtClusterId,
            addressId,
            static_cast<uint8_t>(Flags::Request)};

        return {header};
    }

    /**
     * \brief Create a fireAndForget request message that originates from a proxy and is targetted
     * to a specific skeleton.
     *
     * \param serviceId service ID
     * \param memberId member ID
     * \param serviceInstanceId service instance ID
     * \param srcClusterId source cluster ID
     * \param tgtClusterId target cluster ID
     * \return created message
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
            static_cast<uint8_t>(Flags::FireAndForgetRequest)};

        return {header};
    }

    /**
     * \brief Create a response message that originates from a skeleton and is targetted to a unique
     * proxy.
     *
     * \param header reference to the message header
     * \return created message
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
        Header header{
            serviceId,
            memberId,
            requestId,
            serviceInstanceId,
            srcClusterId,
            tgtClusterId,
            addressId,
            static_cast<uint8_t>(Flags::Response)};

        return {header};
    }

    /**
     * \brief Create an event message without source cluster and target cluster IDs set.
     * \remark After calling this method, you need to manually set the target cluster ID. This
     * allows to use the same message to be sent to several clusters by just adapting the cluster
     * ID.
     *
     * \param serviceId the service ID
     * \param memberId the member ID within the service
     * \param serviceInstanceId the service instance ID
     * \return the created message
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
            static_cast<uint8_t>(Flags::Event)};

        return {header};
    }

    /**
     * \brief Create an error response message that originates from a skeleton and is targetted to a
     * unique proxy, and sets the payload with value \param error.
     *
     * \param header reference to the message header
     * \param error error code to be sent in the payload
     * \return created message
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
        Header header{
            serviceId,
            memberId,
            requestId,
            serviceInstanceId,
            srcClusterId,
            tgtClusterId,
            addressId,
            static_cast<uint8_t>(Flags::Error)};

        Message msg{header};
        msg._payload.error = error;

        return msg;
    }

    /**
     * \brief Check if message is a request.
     *
     * \return true if Flags::Request is active, otherwise returns false.
     */
    bool isRequest() const { return hasActiveFlag(Flags::Request); }

    /**
     * \brief Check if message is a fire and forget request.
     *
     * \return true if Flags::FireAndForgetRequest is active, otherwise returns false.
     */
    bool isFireAndForgetRequest() const { return hasActiveFlag(Flags::FireAndForgetRequest); }

    /**
     * \brief Check if message is a response.
     *
     * \return true if Flags::Response is active, otherwise returns false.
     */
    bool isResponse() const { return hasActiveFlag(Flags::Response); }

    /**
     * \brief Check if message contains an error.
     *
     * \return true if Flags::Error is active, otherwise returns false.
     */
    bool isError() const { return hasActiveFlag(Flags::Error); }

    /**
     * \brief Check if message is an event.
     *
     * \return true if Flags::Event is active, otherwise returns false.
     */
    bool isEvent() const { return hasActiveFlag(Flags::Event); }

    /**
     * \brief Check if message contains a reference to a unique external payload.
     *
     * \return true if Flags::UniqueExternalPayload is active, otherwise returns false.
     */
    bool hasUniqueExternalPayload() const { return hasActiveFlag(Flags::UniqueExternalPayload); }

    /**
     * \brief Check if message contains a reference to a shared external payload.
     *
     * \return true if Flags::SharedExternalPayload is active, otherwise returns false.
     */
    bool hasSharedExternalPayload() const { return hasActiveFlag(Flags::SharedExternalPayload); }

    /**
     * \brief Get the ErrorState value of the message.
     *
     * \return the current ErrorState value if message has error, otherwise returns
     * ErrorState::NoError.
     */
    ErrorState getErrorState() const { return isError() ? _payload.error : ErrorState::NoError; }

private:
    friend class MessageAllocator;

    constexpr Message(Header const& header) : _header(header), _payload() {}

    enum class Flags : uint8_t
    {
        Request               = 0b00000001U, // 1 << 0
        FireAndForgetRequest  = 0b00000010U, // 1 << 1
        Event                 = 0b00000100U, // 1 << 2
        Response              = 0b00001000U, // 1 << 3
        Error                 = 0b00010000U, // 1 << 4
        UniqueExternalPayload = 0b00100000U, // 1 << 5
        SharedExternalPayload = 0b01000000U, // 1 << 6
    };

    /**
     * \brief Gets a constant reference of the object that is stored inside the payload's internal
     * buffer. \remark This method assumes that the user has checked that the message contains the
     * payload in its internal buffer, and not an ExternalHandle object or an error value.
     *
     * \tparam T the generic type which must be trivially copyable and have a size less than the
     * internal payload's size. \return constexpr const T&
     */
    template<typename T>
    constexpr T const& getObjectStoredInPayload() const
    {
        static_assert(
            sizeof(T) <= MAX_PAYLOAD_SIZE, "Size of type T must be smaller than MAX_PAYLOAD_SIZE!");
        static_assert(
            etl::is_trivially_copyable<T>::value, "T must have a trivial copy constructor!");

        // TODO: use construct_object_at const specialization once available in ETL
        // return etl::get_object_at<T const>(_payload.internalBuffer.data());
        return *reinterpret_cast<T const*>(_payload.internalBuffer.data());
    }

    /**
     * \brief Constructs a copy of \param obj inside the payload's internal buffer.
     *
     * \tparam T the generic type which must be trivially copyable and have a size less than the
     * internal payload's size. \param obj
     */
    template<typename T>
    constexpr void constructObjectAtPayload(T const& obj)
    {
        static_assert(
            sizeof(T) <= MAX_PAYLOAD_SIZE, "Size of type T must be smaller than MAX_PAYLOAD_SIZE!");
        static_assert(
            etl::is_trivially_copyable<T>::value, "T must have a trivial copy constructor!");

        unsetFlag(Flags::UniqueExternalPayload);
        unsetFlag(Flags::SharedExternalPayload);
        etl::construct_object_at(_payload.internalBuffer.data(), obj);
    }

    /**
     * \brief Gets a constant reference to the ExternalHandle object that is stored inside the
     * message's payload. \remark This method assumes that the user has checked that the message
     * contains an ExternalHandle object, and not an error value or the actual payload stored inside
     * the internal buffer.
     *
     * \return const ExternalHandle&
     */
    ExternalHandle const& getExternalHandle() const { return _payload.externalHandle; }

    /**
     * \brief Set the payload as an ExternalHandle type which will contain information to where the
     * actual message's payload is stored.
     *
     * \param offset
     * \param size
     * \param isShared
     */
    void setExternalHandle(ptrdiff_t const offset, size_t const size, bool const isShared)
    {
        setFlag(isShared ? Flags::SharedExternalPayload : Flags::UniqueExternalPayload);
        etl::construct_object_at<ExternalHandle>(
            &_payload.externalHandle, ExternalHandle{offset, size});
    }

    /**
     * \brief Checks if a flag is currently active in the flags bitmask.
     *
     * \param flag
     * \return true if \param flag is active, otherwise false.
     */
    constexpr bool hasActiveFlag(Flags const flag) const
    {
        return (_header.flags & static_cast<uint8_t>(flag)) == static_cast<uint8_t>(flag);
    }

    /**
     * \brief Sets the \param flag to active in the flags bitmask.
     *
     * \param flag
     */
    constexpr void setFlag(Flags const flag) { _header.flags |= static_cast<uint8_t>(flag); }

    /**
     * \brief Unsets the \param flag to active in the flags bitmask.
     *
     * \param flag
     */
    constexpr void unsetFlag(Flags const& flag) { _header.flags &= ~static_cast<uint8_t>(flag); }

    Header
        _header; ///< The message header containing the service, method, request and instance ids.
    PayloadType _payload; ///< The message payload which can either store some data
                          ///< constructed in place, and external handle pointing to the
                          ///< place where the actual data is stored or an error value.
};

} // namespace core
} // namespace middleware
