// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/types.h"

#include <etl/array.h>
#include <etl/memory.h>

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
 * - some dispatching information like the source cluster from where the message originates, the
 * target cluster to which the message needs to go and an address ID that is unique to each proxy
 * instance;
 * - a payload which is a union of a buffer, and external handle that contains information to where
 * the actual payload might be stored or an error value.
 */
class Message
{
public:
    /**
     * \brief The message's header
     */
    struct Header
    {
        uint16_t serviceId;
        uint16_t memberId;
        uint16_t requestId;
        uint16_t serviceInstanceId;
    };

    static constexpr size_t MAX_MESSAGE_SIZE = 32U;
    static constexpr size_t MAX_PAYLOAD_SIZE = MAX_MESSAGE_SIZE - sizeof(Header)
                                               - (2 * sizeof(uint8_t)) - sizeof(uint8_t)
                                               - sizeof(uint8_t);

    /**
     * \brief An object that contains information to where the data is stored.
     * \details This object will be one of the possible types of the payload union. Whenever the
     * data that needs to be sent is biffer than MAX_PAYLOAD_SIZE, the data must be allocated
     * externally and the message's payload set to this object. This object has an offset from
     * the beginning of the memory section where the data was stored, the size of the data and a
     * boolean value that is used to indicate if this data is shared by several messages.
     *
     */
    struct ExternalHandle
    {
        constexpr ExternalHandle() = default;

        uint32_t offset{0U}; ///< The offset, from the beginning of the memory region dedicated for
                             ///< storing middleware data, where the message's payload is stored.
        uint32_t size{0U};   ///< The size of the payload that is stored.
        bool isPayloadShared{false}; ///< Tells if the payload is shared by several messages.
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

    /**
     * \brief Default c'tor must remain empty.
     * \remark In case a core is initialized after another core has already sent a message over a
     * queue, the default c'tor of all messages is called again, thus invalidating the first core's
     * message payload. As such the constructor needs to be empty in order to not do any work.
     */
    Message() {}

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
    Header const& getHeader() const { return header_; }

    /**
     * \brief Get the source cluster id.
     *
     * \return uint8_t
     */
    uint8_t getSourceClusterId() const { return srcClusterId_; }

    /**
     * \brief Set the source cluster id.
     *
     * \param clusterId
     */
    void setSourceClusterId(uint8_t const clusterId) { srcClusterId_ = clusterId; }

    /**
     * \brief Get the target cluster id.
     *
     * \return uint8_t
     */
    uint8_t getTargetClusterId() const { return tgtClusterId_; }

    /**
     * \brief Set the target cluster id.
     *
     * \param clusterId
     */
    void setTargetClusterId(uint8_t const clusterId) { tgtClusterId_ = clusterId; }

    /**
     * \brief Get the address id.
     *
     * \return uint8_t
     */
    uint8_t getAddressId() const { return addressId_; }

    /**
     * \brief Set the address id.
     *
     * \param address
     */
    void setAddressId(uint8_t const address) { addressId_ = address; }

    /**
     * \brief Get current active flags.
     *
     * \return currently active flags
     */
    uint8_t getFlags() const { return flags_; }

    /**
     * \brief Create a request message that originates from a proxy and is targetted to a specific
     * skeleton.
     *
     * \param header reference to the message header
     * \param srcClusterId source cluster ID
     * \param tgtClusterId target cluster ID
     * \param addressId unique ID of the target skeleton
     * \return created message
     */
    static constexpr Message createRequest(
        Header const& header,
        uint8_t const srcClusterId,
        uint8_t const tgtClusterId,
        uint8_t const addressId)
    {
        Message msg{header, srcClusterId, tgtClusterId, addressId};
        msg.setFlag_(Flags::SkeletonTarget);
        msg.setFlag_(Flags::HasOutArgs);

        return msg;
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
        Header const header{serviceId, memberId, INVALID_REQUEST_ID, serviceInstanceId};
        Message msg{header, srcClusterId, tgtClusterId};
        msg.setFlag_(Flags::SkeletonTarget);

        return msg;
    }

    /**
     * \brief Create a response message that originates from a skeleton and is targetted to a unique
     * proxy.
     *
     * \param header reference to the message header
     * \param srcClusterId source cluster ID
     * \param tgtClusterId target cluster ID
     * \param addressId unique ID of the target proxy
     * \return created message
     */
    static constexpr Message createResponse(
        Header const& header,
        uint8_t const srcClusterId,
        uint8_t const tgtClusterId,
        uint8_t const addressId)
    {
        Message msg{header, srcClusterId, tgtClusterId, addressId};
        msg.setFlag_(Flags::ProxyTarget);
        msg.setFlag_(Flags::HasOutArgs);

        return msg;
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
        };
        Message msg{header, srcClusterId};
        msg.setFlag_(Flags::ProxyTarget);
        msg.setFlag_(Flags::IsEvent);

        return msg;
    }

    /**
     * \brief Create an error response message that originates from a skeleton and is targetted to a
     * unique proxy, and sets the payload with value \param error.
     *
     * \param header reference to the message header
     * \param srcClusterId source cluster ID
     * \param tgtClusterId target cluster ID
     * \param addressId the unique ID of the target proxy
     * \param error error code to be sent in the payload
     * \return created message
     */
    static constexpr Message createErrorResponse(
        Header const& header,
        uint8_t const srcClusterId,
        uint8_t const tgtClusterId,
        uint8_t const addressId,
        ErrorState const error)
    {
        Message msg{header, srcClusterId, tgtClusterId, addressId};
        msg.setFlag_(Flags::ProxyTarget);
        msg.setFlag_(Flags::HasError);
        msg.payload_.error = error;

        return msg;
    }

    /**
     * \brief Check if message is a proxy target.
     *
     * \return true if Flags::ProxyTarget is active, otherwise returns false.
     */
    bool isProxyTarget() const { return hasActiveFlag_(Flags::ProxyTarget); }

    /**
     * \brief Check if message is a skeleton target.
     *
     * \return true if Flags::SkeletonTarget is active, otherwise returns false.
     */
    bool isSkeletonTarget() const { return hasActiveFlag_(Flags::SkeletonTarget); }

    /**
     * \brief Check if message contains an error.
     *
     * \return true if Flags::HasError is active, otherwise returns false.
     */
    bool hasError() const { return hasActiveFlag_(Flags::HasError); }

    /**
     * \brief Check if message is an event.
     *
     * \return true if Flags::IsEvent is active, otherwise returns false.
     */
    bool isEvent() const { return hasActiveFlag_(Flags::IsEvent); }

    /**
     * \brief Check if message contains output arguments.
     *
     * \return true if Flags::HasOutArgs is active, otherwise returns false.
     */
    bool hasOutArgs() const { return hasActiveFlag_(Flags::HasOutArgs); }

    /**
     * \brief Check if message contains a reference to an external payload.
     *
     * \return true if Flags::HasExternalPayload is active, otherwise returns false.
     */
    bool hasExternalPayload() const { return hasActiveFlag_(Flags::HasExternalPayload); }

    /**
     * \brief Get the ErrorState value of the message.
     *
     * \return the current ErrorState value if message has error, otherwise returns
     * ErrorState::NoError.
     */
    ErrorState getErrorState() const { return hasError() ? payload_.error : ErrorState::NoError; }

private:
    friend class MessageAllocator;

    constexpr Message(
        Header const& header,
        uint8_t const srcClusterId,
        uint8_t const tgtClusterId = INVALID_CLUSTER_ID,
        uint8_t const addressId    = INVALID_ADDRESS_ID)
    : header_(header)
    , srcClusterId_(srcClusterId)
    , tgtClusterId_(tgtClusterId)
    , addressId_(addressId)
    , flags_()
    , payload_()
    {}

    enum class Flags : uint8_t
    {
        ProxyTarget        = 0b00000001U, // 1 << 0
        SkeletonTarget     = 0b00000010U, // 1 << 1
        HasExternalPayload = 0b00000100U, // 1 << 2
        HasError           = 0b0001000U,  // 1 << 3
        IsEvent            = 0b00010000U, // 1 << 4
        HasOutArgs         = 0b00100000U, // 1 << 5
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
    constexpr T const& getObjectStoredInPayload_() const
    {
        static_assert(
            sizeof(T) <= MAX_PAYLOAD_SIZE, "Size of type T must be smaller than MAX_PAYLOAD_SIZE!");
        static_assert(
            etl::is_copy_constructible<T>::value, "T must have a trivial copy constructor!");

        return etl::get_object_at<T const>(payload_.internalBuffer.data());
    }

    /**
     * \brief Constructs a copy of \param obj inside the payload's internal buffer.
     *
     * \tparam T the generic type which must be trivially copyable and have a size less than the
     * internal payload's size. \param obj
     */
    template<typename T>
    constexpr void constructObjectAtPayload_(T const& obj)
    {
        static_assert(
            sizeof(T) <= MAX_PAYLOAD_SIZE, "Size of type T must be smaller than MAX_PAYLOAD_SIZE!");
        static_assert(
            etl::is_copy_constructible<T>::value, "T must have a trivial copy constructor!");

        unsetFlag_(Flags::HasExternalPayload);
        etl::construct_object_at(payload_.internalBuffer.data(), obj);
    }

    /**
     * \brief Get a reference to payload_ interpreted as an ExternalHandle object.
     *
     * \return ExternalHandle&.
     */

    /**
     * \brief Gets a constant reference to the ExternalHandle object that is stored inside the
     * message's payload. \remark This method assumes that the user has checked that the message
     * contains an ExternalHandle object, and not an error value or the actual payload stored inside
     * the internal buffer.
     *
     * \return const ExternalHandle&
     */
    ExternalHandle const& getExternalHandle_() const { return payload_.externalHandle; }

    /**
     * \brief Set the payload as an ExternalHandle type which will contain information to where the
     * actual message's payload is stored.
     *
     * \param handle
     */
    void setExternalHandle_(ExternalHandle const& handle)
    {
        setFlag_(Flags::HasExternalPayload);
        etl::construct_object_at<ExternalHandle>(&payload_.externalHandle, handle);
    }

    /**
     * \brief Checks if a flag is currently active in the flags bitmask.
     *
     * \param flag
     * \return true if \param flag is active, otherwise false.
     */
    constexpr bool hasActiveFlag_(Flags const flag) const
    {
        return (flags_ & static_cast<uint8_t>(flag)) == static_cast<uint8_t>(flag);
    }

    /**
     * \brief Sets the \param flag to active in the flags bitmask.
     *
     * \param flag
     */
    constexpr void setFlag_(Flags const flag) { flags_ |= static_cast<uint8_t>(flag); }

    /**
     * \brief Unsets the \param flag to active in the flags bitmask.
     *
     * \param flag
     */
    constexpr void unsetFlag_(Flags const& flag) { flags_ &= ~static_cast<uint8_t>(flag); }

    Header
        header_; ///< The message header containing the service, method, request and instance ids.
    uint8_t srcClusterId_; ///< The source cluster id.
    uint8_t tgtClusterId_; ///< The target cluster id.
    uint8_t addressId_;    ///< The proxy unique id.
    uint8_t flags_;        ///< Flags bitmask which contains a combination of Flags values.
    PayloadType payload_;  ///< The message payload which can either store some data
                           ///< constructed in place, and external handle pointing to the
                           ///< place where the actual data is stored or an error value.
};

} // namespace core
} // namespace middleware

constexpr bool operator==(
    middleware::core::Message::ExternalHandle const& lhs,
    middleware::core::Message::ExternalHandle const& rhs)
{
    return (lhs.offset == rhs.offset) && (lhs.size == rhs.size)
           && (lhs.isPayloadShared == rhs.isPayloadShared);
}

constexpr bool operator!=(
    middleware::core::Message::ExternalHandle const& lhs,
    middleware::core::Message::ExternalHandle const& rhs)
{
    return !(lhs == rhs);
}
